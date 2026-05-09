import { networkInterfaces } from "node:os";

const localDevOrigins = Array.from(
  new Set(
    Object.values(networkInterfaces())
      .flat()
      .filter((entry) => entry && entry.family === "IPv4" && !entry.internal)
      .map((entry) => entry.address)
  )
);

/** @type {import('next').NextConfig} */
const nextConfig = {
  allowedDevOrigins: ["127.0.0.1", "localhost", ...localDevOrigins],
  outputFileTracingIncludes: {
    "/api/compile": ["./minic"]
  }
};

export default nextConfig;
