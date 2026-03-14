import { defineConfig } from "astro/config";

const repository = process.env.GITHUB_REPOSITORY ?? "";
const [owner, repo] = repository.split("/");
const isProjectPage = Boolean(repo) && !repo.endsWith(".github.io");
const base = process.env.GITHUB_ACTIONS && isProjectPage ? `/${repo}` : "/";
const site =
  owner && repo
    ? `https://${owner}.github.io${isProjectPage ? `/${repo}` : ""}`
    : "https://example.github.io";

export default defineConfig({
  site,
  base,
});
