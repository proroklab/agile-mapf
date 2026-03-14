export function toYouTubeEmbedUrl(rawUrl?: string | null): string | null {
  if (!rawUrl) {
    return null;
  }

  try {
    const parsedUrl = new URL(rawUrl);
    const hostname = parsedUrl.hostname.toLowerCase();

    if (hostname.includes("youtube.com") && parsedUrl.pathname.startsWith("/embed/")) {
      const embedId = parsedUrl.pathname.split("/").filter(Boolean).at(-1);
      return embedId && embedId.length >= 6 ? `https://www.youtube.com/embed/${embedId}` : null;
    }

    if (hostname.includes("youtube.com")) {
      const watchId = parsedUrl.searchParams.get("v");
      return watchId && watchId.length >= 6 ? `https://www.youtube.com/embed/${watchId}` : null;
    }

    if (hostname.includes("youtu.be")) {
      const shortId = parsedUrl.pathname.replace("/", "");
      return shortId && shortId.length >= 6 ? `https://www.youtube.com/embed/${shortId}` : null;
    }
  } catch {
    return null;
  }

  return null;
}
