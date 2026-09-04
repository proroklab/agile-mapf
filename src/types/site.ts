import type { CollectionEntry } from "astro:content";

export type SiteContent = CollectionEntry<"site">["data"];
export type HeroContent = SiteContent["hero"];
export type AbstractSectionContent = SiteContent["sections"]["abstract"];
export type NewsSectionContent = SiteContent["sections"]["news"];
export type PaperDiveSectionContent = SiteContent["sections"]["paperDive"];
export type MainVideoSectionContent = SiteContent["sections"]["mainVideo"];
export type MethodSectionContent = SiteContent["sections"]["method"];
export type VideosSectionContent = SiteContent["sections"]["videos"];
export type CitationSectionContent = SiteContent["sections"]["citation"];
export type ContactSectionContent = SiteContent["sections"]["contact"];
