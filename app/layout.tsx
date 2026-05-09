import type { Metadata } from "next";
import "./globals.css";

export const metadata: Metadata = {
  title: "Mini C Compiler Lab",
  description: "Interactive Mini C compiler frontend"
};

export default function RootLayout({
  children
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="en" suppressHydrationWarning>
      <body suppressHydrationWarning>{children}</body>
    </html>
  );
}
