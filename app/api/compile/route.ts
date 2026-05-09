import { spawn } from "node:child_process";
import { randomUUID } from "node:crypto";
import { access, unlink, writeFile } from "node:fs/promises";
import { constants } from "node:fs";
import { tmpdir } from "node:os";
import path from "node:path";
import { NextResponse } from "next/server";

export const runtime = "nodejs";
export const dynamic = "force-dynamic";

type StageOutput = {
  tokens: string;
  parseTree: string;
  astJson: AstNode | null;
  symbolTable: string;
  ir: string;
  finalCode: string;
  diagnostics: string;
  result: string;
};

type CompilerPhase = "lexical" | "syntax" | "semantic" | "complete" | "unknown";
type StageKey = Exclude<keyof StageOutput, "astJson">;

type AstNode = {
  kind: string;
  value: string;
  children: AstNode[];
};

type CompilerRun = {
  accepted: boolean;
  exitCode: number | null;
  optimizationEnabled: boolean;
  timeNs: string;
  timeMs: number;
  stdout: string;
  stderr: string;
  stoppedAt: string | null;
  caughtPhase: CompilerPhase;
  availableStages: StageKey[];
  stages: StageOutput;
};

const SECTION_HEADINGS = [
  "TOKENS",
  "SYMBOL TABLE",
  "ABSTRACT SYNTAX TREE",
  "AST JSON",
  "THREE ADDRESS CODE",
  "DIAGNOSTICS"
];

function findCompilerPath() {
  const candidates = [
    process.env.MINIC_PATH,
    path.join(/* turbopackIgnore: true */ process.cwd(), "minic"),
    path.join(/* turbopackIgnore: true */ process.cwd(), "..", "minic")
  ].filter(Boolean) as string[];

  return candidates;
}

async function resolveCompilerPath() {
  for (const candidate of findCompilerPath()) {
    try {
      await access(candidate, constants.X_OK);
      return candidate;
    } catch {
      // Try the next known location.
    }
  }
  throw new Error("Could not find executable compiler binary. Run `make` before starting the web app.");
}

function section(stdout: string, heading: string) {
  const lines = stdout.replace(/\r\n/g, "\n").split("\n");
  const start = lines.findIndex((line) => line.trim() === heading);
  if (start === -1) return "";

  let end = lines.length;
  for (let i = start + 1; i < lines.length; i += 1) {
    const trimmed = lines[i].trim();
    if (
      SECTION_HEADINGS.includes(trimmed) ||
      trimmed.startsWith("Stopped at:") ||
      trimmed.startsWith("Result:")
    ) {
      end = i;
      break;
    }
  }

  return lines.slice(start + 1, end).join("\n").trim();
}

function resultLine(stdout: string) {
  return stdout
    .replace(/\r\n/g, "\n")
    .split("\n")
    .find((line) => line.trim().startsWith("Result:"))
    ?.trim() ?? "Result: unknown";
}

function stoppedAtLine(stdout: string) {
  return stdout
    .replace(/\r\n/g, "\n")
    .split("\n")
    .find((line) => line.trim().startsWith("Stopped at:"))
    ?.trim()
    .replace(/^Stopped at:\s*/, "") ?? null;
}

function phaseFromStoppedAt(stoppedAt: string | null, accepted: boolean): CompilerPhase {
  if (accepted) return "complete";
  if (!stoppedAt) return "unknown";
  if (/lexical/i.test(stoppedAt)) return "lexical";
  if (/syntax/i.test(stoppedAt)) return "syntax";
  if (/semantic/i.test(stoppedAt)) return "semantic";
  return "unknown";
}

function availableStagesFor(phase: CompilerPhase): StageKey[] {
  if (phase === "complete") {
    return ["tokens", "parseTree", "symbolTable", "ir", "finalCode", "diagnostics", "result"];
  }
  if (phase === "semantic") {
    return ["tokens", "parseTree", "symbolTable", "diagnostics", "result"];
  }
  if (phase === "syntax" || phase === "lexical") {
    return ["tokens", "diagnostics", "result"];
  }
  return ["diagnostics", "result"];
}

function parseStages(stdout: string, accepted: boolean, phase: CompilerPhase): StageOutput {
  const tac = section(stdout, "THREE ADDRESS CODE");
  const astJsonText = section(stdout, "AST JSON");
  let astJson: AstNode | null = null;

  if (astJsonText) {
    try {
      astJson = JSON.parse(astJsonText) as AstNode;
    } catch {
      astJson = null;
    }
  }

  return {
    tokens: section(stdout, "TOKENS"),
    parseTree:
      section(stdout, "ABSTRACT SYNTAX TREE") ||
      (accepted ? "AST output is empty." : `Stopped before AST output at ${phase}.`),
    astJson,
    symbolTable: section(stdout, "SYMBOL TABLE") || `Stopped before symbol table output at ${phase}.`,
    ir: tac || `Stopped before IR/TAC output at ${phase}.`,
    finalCode: tac || `Stopped before final code output at ${phase}.`,
    diagnostics: section(stdout, "DIAGNOSTICS"),
    result: resultLine(stdout)
  };
}

async function runCompiler(sourcePath: string, optimizationEnabled: boolean): Promise<CompilerRun> {
  const compilerPath = await resolveCompilerPath();
  const args = [sourcePath, "--tokens", "--symbols", "--ast", "--ast-json", "--tac"];
  if (!optimizationEnabled) args.push("--no-opt");

  const startedAt = process.hrtime.bigint();

  const processResult = await new Promise<{
    stdout: string;
    stderr: string;
    exitCode: number | null;
  }>((resolve, reject) => {
    const child = spawn(compilerPath, args, {
      cwd: process.cwd(),
      windowsHide: true
    });

    let stdout = "";
    let stderr = "";
    const timeout = setTimeout(() => {
      child.kill("SIGTERM");
      reject(new Error("Compiler timed out after 10 seconds."));
    }, 10_000);

    child.stdout.setEncoding("utf8");
    child.stderr.setEncoding("utf8");
    child.stdout.on("data", (chunk) => {
      stdout += chunk;
    });
    child.stderr.on("data", (chunk) => {
      stderr += chunk;
    });
    child.on("error", (error) => {
      clearTimeout(timeout);
      reject(error);
    });
    child.on("close", (exitCode) => {
      clearTimeout(timeout);
      resolve({ stdout, stderr, exitCode });
    });
  });

  const endedAt = process.hrtime.bigint();
  const duration = endedAt - startedAt;
  const accepted = processResult.exitCode === 0 && /Result:\s+accepted/.test(processResult.stdout);
  const stoppedAt = stoppedAtLine(processResult.stdout);
  const caughtPhase = phaseFromStoppedAt(stoppedAt, accepted);

  return {
    accepted,
    exitCode: processResult.exitCode,
    optimizationEnabled,
    timeNs: duration.toString(),
    timeMs: Number(duration) / 1_000_000,
    stdout: processResult.stdout,
    stderr: processResult.stderr,
    stoppedAt,
    caughtPhase,
    availableStages: availableStagesFor(caughtPhase),
    stages: parseStages(processResult.stdout, accepted, caughtPhase)
  };
}

export async function POST(request: Request) {
  let sourcePath: string | null = null;

  try {
    const body = (await request.json()) as { source?: unknown; optimize?: unknown };
    const source = typeof body.source === "string" ? body.source : "";
    const optimize = typeof body.optimize === "boolean" ? body.optimize : true;

    if (!source.trim()) {
      return NextResponse.json({ error: "Source code is required." }, { status: 400 });
    }

    sourcePath = path.join(tmpdir(), `minic-${randomUUID()}.c`);
    await writeFile(sourcePath, source, "utf8");

    const [unoptimized, optimized] = await Promise.all([
      runCompiler(sourcePath, false),
      runCompiler(sourcePath, true)
    ]);

    const selected = optimize ? optimized : unoptimized;
    const differenceNs = BigInt(unoptimized.timeNs) - BigInt(optimized.timeNs);

    return NextResponse.json({
      accepted: selected.accepted,
      selectedMode: optimize ? "optimized" : "unoptimized",
      unoptimized,
      optimized,
      differenceNs: differenceNs.toString(),
      differenceMs: Number(differenceNs) / 1_000_000
    });
  } catch (error) {
    const message = error instanceof Error ? error.message : "Unknown compiler error.";
    return NextResponse.json({ error: message }, { status: 500 });
  } finally {
    if (sourcePath) {
      await unlink(sourcePath).catch(() => undefined);
    }
  }
}
