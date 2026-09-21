import type { PluginSimple } from 'markdown-it'

/**
 * 代码卡语言徽章文案美化（对齐 tamcpp code-label-plugin）：
 * 把 Shiki 输出的 <span class="lang">cpp</span> 等小写机读名换成可读名。
 * 只改可见 label，VitePress 的 copy/lang/pre 兄弟链不动。
 *
 * 映射按全站语言分布取舍（cpp 2226 / qml 134 / cmake 62 / bash 37 / css 27 /
 * xml 12 / text 8 / json 4 …，2026-09 统计）；mermaid 不进 fence 渲染链，不列。
 * 新语言要美化只改 LANG_LABELS 一张表。
 */
const LANG_LABELS: Record<string, string> = {
  cpp: 'C++',
  c: 'C',
  qml: 'QML',
  cmake: 'CMake',
  bash: 'Shell',
  sh: 'Shell',
  xml: 'XML',
  json: 'JSON',
  javascript: 'JS',
  html: 'HTML',
  css: 'CSS',
  text: '文本',
  glsl: 'GLSL',
  ini: 'INI',
}

export const codeLabelPlugin: PluginSimple = (md) => {
  const fence = md.renderer.rules.fence
  if (!fence) return
  md.renderer.rules.fence = (...args) => {
    const html = fence(...args)
    return html.replace(/(<span class="lang">)([a-z]+)(<\/span>)/g, (m, open, lang, close) =>
      LANG_LABELS[lang] ? open + LANG_LABELS[lang] + close : m,
    )
  }
}
