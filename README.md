# Agile MAPF Project Page

Astro-based research project page template focused on media-rich robotics publications.

## Quick Start

```bash
npm install
npm run dev
```

Build for production:

```bash
npm run build
npm run preview
```

## Reuse in Another Project

1. Replace site content in `src/content/site.yaml`.
2. Update `seo` fields in `src/content/site.yaml` (`description`, `ogImage`, `ogType`, `twitterCard`).
3. Replace media under `public/images/` and `public/media/`.
4. Run `npm run build` to validate schema and rendering.

## Project Structure

- Content source of truth: `src/content/site.yaml`
- Content schema validation: `src/content.config.ts`
- Page entrypoint: `src/pages/index.astro`
- Reusable sections: `src/components/sections/*.astro`
- Shared styles: `src/styles/global.css`
- Utility functions: `src/lib/*.ts`
- Shared content types: `src/types/site.ts`

## Content Model Notes

`src/content.config.ts` enforces required section structure and gallery accessibility fields.
If YAML fields are missing or malformed, build fails early.

## Deployment (GitHub Pages)

This repository includes a ready-to-use workflow at `.github/workflows/deploy-pages.yml`.

### Prerequisites

- You have permission to push to the repository's `main` branch.
- GitHub Actions is enabled for the repository.

### One-time setup

1. Open your repository on GitHub.
2. Go to `Settings` -> `Pages`.
3. Set `Source` to `GitHub Actions`.
4. Save the setting.

### Deploy steps

1. Commit and push your changes to `main`.
2. Open the `Actions` tab and select `Deploy to GitHub Pages`.
3. Confirm the workflow run completes successfully (`build` then `deploy` jobs).
4. Open the deployed URL shown in the `deploy` job summary.

### Manual redeploy (without new commit)

1. Open `Actions` -> `Deploy to GitHub Pages`.
2. Click `Run workflow`.
3. Select branch `main` and start the run.

### How the URL/base path is handled

- User/organization pages (`<owner>.github.io`) are served from `/`.
- Project pages are served from `/<repo>`.
- This is set automatically in `astro.config.mjs` using `GITHUB_REPOSITORY`.

## License

This repository is licensed under the MIT License.
See [LICENSE](./LICENSE) for details.
