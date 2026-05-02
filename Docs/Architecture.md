# MarkdownSlate Architecture

## Overview

MarkdownSlate is a UE5.7 plugin that parses Markdown into AST, builds render trees, and displays content via Slate/UMG widgets.

## Module Structure

| Module | Type | Purpose |
|---|---|---|
| `MarkdownSlateMD4C` | Runtime | Wraps third-party md4c C parser |
| `MarkdownSlate` | Runtime | AST, Parser, RenderBuilder, SlateRenderer, UMG Widget |
| `MarkdownSlateEditor` | Editor | Asset factories, AssetTypeActions, Preview Tab, ToolMenus |

## Data Flow

```
Markdown Text (FString)
  → FMarkdownParser::Parse() → AST (FMarkdownAstNode tree)
  → FMarkdownRenderBuilder::Build() → RenderNode tree (FMarkdownRenderNode)
  → FMarkdownSlateRenderer::Render() → SWidget (Slate UI)
  → UMarkdownWidget → UMG Widget (Blueprint)
```

## Rendering Pipeline

1. **Parser** wraps md4c callbacks to build AST nodes (`FMarkdownBlockNode`, `FMarkdownSpanNode`, `FMarkdownTextNode`)
2. **RenderBuilder** converts AST to flattened `FMarkdownRenderNode` tree with inline ordering
3. **Renderer** creates Slate widgets: `SWrapBox` for inline flow, `SGridPanel` for tables, `SOverlay` for strikethrough
4. **Emoji Scanner** detects emoji codepoints and optionally renders via Twemoji texture atlas
5. **Render Cache** caches parsed RenderNode trees keyed by source hash + theme hash + width bucket

## Theme System

`UMarkdownThemeAsset` (DataAsset) holds all visual configuration. `CopyThemeToConfig()` copies to `FMarkdownSlateThemeConfig` at render time. Theme edits auto-sync via `PostEditChangeProperty` + `FCoreUObjectDelegates`.
