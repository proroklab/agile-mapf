import { defineCollection, z } from "astro:content";
import { glob } from "astro/loaders";

const heroSchema = z.object({
  title: z.string(),
  venue: z.string().optional(),
  authors: z.array(
    z.object({
      name: z.string(),
      marks: z.string(),
      url: z.string().url().optional(),
    }),
  ),
  affiliations: z.array(z.string()),
  notes: z.array(z.string()),
  ctas: z.array(
    z.object({
      label: z.string(),
      href: z.string(),
      style: z.enum(["solid", "ghost"]),
    }),
  ),
});

const sectionsSchema = z.object({
  abstract: z.object({
    id: z.string(),
    title: z.string(),
    body: z.string(),
  }),
  news: z.object({
    id: z.string(),
    title: z.string(),
    items: z.array(
      z.object({
        date: z.string(),
        text: z.string(),
        url: z.string().url().optional(),
      }),
    ),
  }),
  method: z.object({
    id: z.string(),
    title: z.string(),
    video: z
      .object({
        src: z.string(),
        title: z.string(),
        caption: z.string().optional(),
      })
      .optional(),
  }),
  mainVideo: z.object({
    id: z.string(),
    title: z.string(),
    url: z.string().optional(),
  }),
  videos: z.object({
    id: z.string(),
    title: z.string(),
    items: z.array(
      z.object({
        type: z.enum(["image", "video"]),
        src: z.string(),
        title: z.string(),
      }),
    ),
  }),
  citation: z.object({
    id: z.string(),
    title: z.string(),
    bibtex: z.string(),
  }),
  contact: z.object({
    id: z.string(),
    title: z.string(),
    prefix: z.string(),
    emails: z.array(z.string().email()),
  }),
});

const site = defineCollection({
  loader: glob({ pattern: "site.yaml", base: "./src/content" }),
  schema: z.object({
    pageTitle: z.string(),
    seo: z.object({
      description: z.string(),
      ogImage: z.string(),
      ogType: z.enum(["website", "article"]).default("website"),
      twitterCard: z.enum(["summary", "summary_large_image"]).default("summary_large_image"),
    }),
    hero: heroSchema,
    sections: sectionsSchema,
  }),
});

export const collections = { site };
