# MarkdownSlate

Markdown → AST → RenderNode → Slate/UMG 的 UE5.7 插件。

## 功能

- CommonMark + GFM (表格、任务列表、删除线) 解析
- 内联样式：**粗体**、*斜体*、`行内代码`、[链接]()、~~删除线~~
- 标题、段落、引用块、有序/无序列表、代码块、表格
- Emoji / Twemoji 纹理图集渲染
- 流式文本追加
- 渲染缓存
- UMG Widget (`UMarkdownWidget`) + 蓝图可编辑 Theme Asset
- Editor 工具链：.md 导入、主题工厂、预览标签页

## 快速开始

1. 启用插件 `MarkdownSlate`
2. 创建 `UMarkdownThemeAsset`（Content Browser → Miscellaneous → Data Asset → MarkdownThemeAsset）
3. 在 UMG 中添加 `Markdown View` 控件
4. 设置 `Theme` 和 `MarkdownText`

## 模块

| 模块 | 类型 |
|---|---|
| MarkdownSlateMD4C | Runtime (md4c wrapper) |
| MarkdownSlate | Runtime (Parser, Render, UMG) |
| MarkdownSlateEditor | Editor (Assets, Preview) |

## 第三方

- [md4c](https://github.com/mity/md4c) — MIT
- [Twemoji](https://github.com/twitter/twemoji) — CC-BY-4.0

## License

MIT — see [LICENSE](./LICENSE)
