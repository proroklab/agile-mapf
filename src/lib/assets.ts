const EXTERNAL_URL_PATTERN = /^(?:[a-z]+:)?\/\//i;

export function toPublicAssetUrl(assetPath: string): string {
  if (
    !assetPath ||
    EXTERNAL_URL_PATTERN.test(assetPath) ||
    assetPath.startsWith("data:")
  ) {
    return assetPath;
  }

  const baseUrl = import.meta.env.BASE_URL ?? "/";
  const normalizedBaseUrl = baseUrl.endsWith("/") ? baseUrl : `${baseUrl}/`;
  const normalizedAssetPath = assetPath.startsWith("/")
    ? assetPath.slice(1)
    : assetPath;
  return `${normalizedBaseUrl}${normalizedAssetPath}`;
}
