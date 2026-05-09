import Link from "next/link";
import {
  innovationCapabilities,
  supportedCapabilities,
  unsupportedCapabilities
} from "../project-content";

export default function FeaturesPage() {
  return (
    <main className="shell infoShell">
      <section className="infoPage">
        <div className="infoHero">
          <div className="infoHeroCopy">
            <p className="eyebrow">Mini C Compiler</p>
            <h1>Compiler Features</h1>
            <p className="infoLead">
              This subset compiler supports the required Compiler Construction pipeline and the
              additional innovation work integrated into the frontend demo.
            </p>
          </div>
          <div className="infoActions">
            <Link className="navButton subtle" href="/">
              Back to Compiler
            </Link>
            <Link className="navButton" href="/about">
              Course & Team
            </Link>
          </div>
        </div>

        <div className="featureGrid">
          <section className="featureSection supported">
            <div className="featureSectionHeader">
              <span>Project Requirements</span>
              <strong>Supported</strong>
            </div>
            <ul className="featureList">
              {supportedCapabilities.map((capability) => (
                <li key={capability}>{capability}</li>
              ))}
            </ul>
          </section>

          <section className="featureSection innovation">
            <div className="featureSectionHeader">
              <span>Innovation Work</span>
              <strong>Additional Capabilities</strong>
            </div>
            <ul className="featureList">
              {innovationCapabilities.map((capability) => (
                <li key={capability}>{capability}</li>
              ))}
            </ul>
          </section>
        </div>

        <section className="featureSection unsupported">
          <div className="featureSectionHeader">
            <span>Out of Scope</span>
            <strong>Not Supported</strong>
          </div>
          <p className="sectionIntro">
            This project targets a practical Mini C subset rather than the full C language.
          </p>
          <ul className="featureList">
            {unsupportedCapabilities.map((capability) => (
              <li key={capability}>{capability}</li>
            ))}
          </ul>
        </section>
      </section>
    </main>
  );
}
