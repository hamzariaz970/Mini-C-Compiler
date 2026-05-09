"use client";

import Link from "next/link";
import { type UIEvent, useMemo, useRef, useState } from "react";

type AstNode = {
  kind: string;
  value: string;
  children: AstNode[];
};

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

type CompilerRun = {
  accepted: boolean;
  exitCode: number | null;
  optimizationEnabled: boolean;
  timeNs: string;
  timeMs: number;
  stdout: string;
  stderr: string;
  stoppedAt: string | null;
  caughtPhase: "lexical" | "syntax" | "semantic" | "complete" | "unknown";
  availableStages: StageKey[];
  stages: StageOutput;
};

type CompileResponse = {
  accepted: boolean;
  selectedMode: "optimized" | "unoptimized";
  unoptimized: CompilerRun;
  optimized: CompilerRun;
  differenceNs: string;
  differenceMs: number;
};

type StageKey = Exclude<keyof StageOutput, "astJson">;

const sampleSource = `int square(int n) {
    return n * n;
}

int main() {
    int x = 2 + 3 * 4;
    int y = square(x);
    return y;

    y = y + 100;
    return y;
}
`;

const stages: Array<{ key: StageKey; label: string }> = [
  { key: "tokens", label: "Lexemes" },
  { key: "parseTree", label: "Parse / AST" },
  { key: "symbolTable", label: "Symbols" },
  { key: "ir", label: "IR" },
  { key: "finalCode", label: "Final Code" },
  { key: "diagnostics", label: "Diagnostics" },
  { key: "result", label: "Result" }
];

function formatNs(value: string) {
  const number = BigInt(value);
  return `${number.toLocaleString()} ns`;
}

function signedNs(value: string) {
  const number = BigInt(value);
  const prefix = number > 0n ? "+" : "";
  return `${prefix}${number.toLocaleString()} ns`;
}

function selectedRun(result: CompileResponse | null) {
  if (!result) return null;
  return result.selectedMode === "optimized" ? result.optimized : result.unoptimized;
}

export default function Home() {
  const [source, setSource] = useState(sampleSource);
  const [optimize, setOptimize] = useState(true);
  const [activeStage, setActiveStage] = useState<StageKey>("tokens");
  const [result, setResult] = useState<CompileResponse | null>(null);
  const [error, setError] = useState("");
  const [loading, setLoading] = useState(false);
  const lineNumberRef = useRef<HTMLDivElement>(null);

  const activeRun = selectedRun(result);
  const activeText = activeRun?.stages[activeStage] || "No output yet.";
  const activeAst = activeRun?.stages.astJson ?? null;
  const lineNumbers = useMemo(() => source.split("\n").map((_, index) => index + 1), [source]);
  const availableStages = activeRun?.availableStages ?? stages.map((stage) => stage.key);
  const activeStageAvailable = availableStages.includes(activeStage);
  const showCompare = Boolean(
    result &&
      (result.unoptimized.availableStages.includes("ir") || result.optimized.availableStages.includes("ir"))
  );

  const statusText = useMemo(() => {
    if (!result) return "Ready";
    return result.accepted ? "Accepted" : "Rejected";
  }, [result]);

  const pipelineMessage = useMemo(() => {
    if (!activeRun) return null;
    if (activeRun.accepted) {
      return {
        tone: "success",
        title: "Pipeline complete",
        detail: "Lexical, syntax, semantic, IR, and final TAC stages all ran."
      };
    }

    const phase = activeRun.stoppedAt ?? "Compiler";
    return {
      tone: "danger",
      title: `Caught by ${phase}`,
      detail: "Compilation stopped there, so later compiler stages were not run."
    };
  }, [activeRun]);

  function syncLineNumbers(event: UIEvent<HTMLTextAreaElement>) {
    if (lineNumberRef.current) {
      lineNumberRef.current.scrollTop = event.currentTarget.scrollTop;
    }
  }

  async function compile() {
    setLoading(true);
    setError("");

    try {
      const response = await fetch("/api/compile", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ source, optimize })
      });

      const data = await response.json();
      if (!response.ok) {
        throw new Error(data.error || "Compile failed.");
      }

      setResult(data);
      setActiveStage(data.accepted ? "tokens" : "diagnostics");
    } catch (compileError) {
      setError(compileError instanceof Error ? compileError.message : "Compile failed.");
    } finally {
      setLoading(false);
    }
  }

  return (
    <main className="shell">
      <section className="workspace">
        <div className="topbar">
          <div>
            <div className="titleRow">
              <h1>Mini C Compiler</h1>
              <Link className="navButton" href="/features">
                Features
              </Link>
              <Link className="navButton subtle" href="/about">
                Course & Team
              </Link>
            </div>
            <p>{statusText}</p>
          </div>
          <div className="actions">
            <label className="switch">
              <input
                type="checkbox"
                checked={optimize}
                onChange={(event) => setOptimize(event.target.checked)}
              />
              <span aria-hidden="true" />
              Optimize
            </label>
            <button className="secondary" type="button" onClick={() => setSource(sampleSource)}>
              Sample
            </button>
            <button className="primary" type="button" onClick={compile} disabled={loading}>
              {loading ? "Compiling" : "Compile"}
            </button>
          </div>
        </div>

        <div className="panes">
          <section className="editorPane" aria-label="Source code editor">
            <div className="paneHeader">
              <span>Source</span>
              <span>{source.split("\n").length} lines</span>
            </div>
            <div className="codeEditor">
              <div className="lineNumbers" ref={lineNumberRef} aria-hidden="true">
                {lineNumbers.map((line) => (
                  <span key={line}>{line}</span>
                ))}
              </div>
              <textarea
                spellCheck={false}
                value={source}
                onChange={(event) => setSource(event.target.value)}
                onScroll={syncLineNumbers}
              />
            </div>
          </section>

          <section className="outputPane" aria-label="Compiler output">
            <div className="metrics">
              <Metric
                label="Without Opt"
                value={result ? formatNs(result.unoptimized.timeNs) : "0 ns"}
              />
              <Metric
                label="With Opt"
                value={result ? formatNs(result.optimized.timeNs) : "0 ns"}
              />
              <Metric
                label="Delta"
                value={result ? signedNs(result.differenceNs) : "0 ns"}
                tone={result && BigInt(result.differenceNs) > 0n ? "good" : "plain"}
              />
            </div>

            {pipelineMessage ? (
              <div className={`pipelineBanner ${pipelineMessage.tone}`}>
                <strong>{pipelineMessage.title}</strong>
                <span>{pipelineMessage.detail}</span>
              </div>
            ) : null}

            <div className="tabs" role="tablist" aria-label="Compiler stages">
              {stages.map((stage) => {
                const disabled = !availableStages.includes(stage.key);

                return (
                  <button
                    key={stage.key}
                    className={[
                      "tab",
                      activeStage === stage.key ? "active" : "",
                      disabled ? "disabled" : ""
                    ].join(" ").trim()}
                    type="button"
                    onClick={() => setActiveStage(stage.key)}
                    disabled={disabled}
                    title={disabled ? "This stage did not run because compilation stopped earlier." : undefined}
                  >
                    {stage.label}
                  </button>
                );
              })}
            </div>

            {activeStage === "parseTree" && activeAst && activeStageAvailable && !error ? (
              <AstViewer root={activeAst} />
            ) : (
              <pre className="stageOutput">
                {error ||
                  (activeStageAvailable
                    ? activeText
                    : "This compiler stage did not run because an earlier stage rejected the program.")}
              </pre>
            )}

            {showCompare && result ? (
              <div className="codeCompare">
                <div>
                  <div className="paneHeader compact">
                    <span>Unoptimized TAC</span>
                    <span>{formatNs(result.unoptimized.timeNs)}</span>
                  </div>
                  <pre>{result.unoptimized.stages.ir || "No TAC emitted."}</pre>
                </div>
                <div>
                  <div className="paneHeader compact">
                    <span>Optimized TAC</span>
                    <span>{formatNs(result.optimized.timeNs)}</span>
                  </div>
                  <pre>{result.optimized.stages.ir || "No TAC emitted."}</pre>
                </div>
              </div>
            ) : null}
          </section>
        </div>
      </section>
    </main>
  );
}

function AstViewer({ root }: { root: AstNode }) {
  return (
    <div className="astPanel">
      <AstBranch node={root} depth={0} />
    </div>
  );
}

function AstBranch({ node, depth }: { node: AstNode; depth: number }) {
  const [open, setOpen] = useState(depth < 3);
  const hasChildren = node.children.length > 0;
  const label = node.value ? `${node.kind}: ${node.value}` : node.kind;

  return (
    <div className="astBranch">
      <button
        className={hasChildren ? "astNode expandable" : "astNode"}
        type="button"
        onClick={() => hasChildren && setOpen((current) => !current)}
        aria-expanded={hasChildren ? open : undefined}
        style={{ paddingLeft: `${10 + depth * 18}px` }}
      >
        <span className="astToggle" aria-hidden="true">
          {hasChildren ? (open ? "v" : ">") : ""}
        </span>
        <span className="astKind">{node.kind}</span>
        {node.value ? <span className="astValue">{node.value}</span> : null}
      </button>

      {hasChildren && open ? (
        <div className="astChildren">
          {node.children.map((child, index) => (
            <AstBranch key={`${child.kind}-${child.value}-${depth}-${index}`} node={child} depth={depth + 1} />
          ))}
        </div>
      ) : null}
    </div>
  );
}

function Metric({
  label,
  value,
  tone = "plain"
}: {
  label: string;
  value: string;
  tone?: "plain" | "good";
}) {
  return (
    <div className={`metric ${tone}`}>
      <span>{label}</span>
      <strong>{value}</strong>
    </div>
  );
}
