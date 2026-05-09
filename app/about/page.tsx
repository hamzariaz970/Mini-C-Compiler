import Link from "next/link";
import { contributors } from "../project-content";

export default function AboutPage() {
  return (
    <main className="shell infoShell">
      <section className="aboutPage">
        <div className="courseCard">
          <p className="eyebrow">Compiler Construction</p>
          <h1>This project was part of Compiler Construction Course, developed by BSCS 12-C.</h1>
          <div className="infoActions centered">
            <Link className="navButton subtle" href="/features">
              Back to Features
            </Link>
            <Link className="navButton" href="/">
              Open Compiler
            </Link>
          </div>
        </div>

        <section className="contributorsSection">
          <div className="featureSectionHeader">
            <span>Contributors</span>
            <strong>Developers</strong>
          </div>
          <p className="sectionIntro">
            This project was made as part of the <strong>Compiler Construction (CS-351)</strong>{" "}
            Course.
          </p>
          <ul className="contributorsList">
            {contributors.map((contributor) => (
              <li key={contributor.email}>
                <a href={contributor.linkedin} target="_blank" rel="noreferrer">
                  {contributor.name}
                </a>
                <span>{contributor.email}</span>
              </li>
            ))}
          </ul>
        </section>
      </section>
    </main>
  );
}
