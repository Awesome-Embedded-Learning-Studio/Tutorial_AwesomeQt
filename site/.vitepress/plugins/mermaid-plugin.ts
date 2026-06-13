import type { PluginSimple } from 'markdown-it'
import type MarkdownIt from 'markdown-it'

function escapeHtml(s: string): string {
  return s
    .replaceAll('&', '&amp;')
    .replaceAll('<', '&lt;')
    .replaceAll('>', '&gt;')
    .replaceAll('"', '&quot;')
    .replaceAll("'", '&#39;')
}

/**
 * 把 ```mermaid 围栏改写成客户端能识别的占位 div。
 *
 * 为什么要抢在 Shiki 之前：VitePress 通过覆盖 md.renderer.rules.fence 让 Shiki
 * 接管所有 fence 渲染。这里在 core ruler 阶段（tokenize 之后、render 之前）把
 * mermaid 围栏的 token.type 改成自定义类型，渲染时分发键就不再是 'fence'，
 * Shiki 永远匹配不到它——否则图表源码会被当成普通代码块高亮。
 */
export const mermaidPlugin: PluginSimple = (md: MarkdownIt) => {
  md.core.ruler.push('mermaid_block', (state) => {
    for (let i = 0; i < state.tokens.length; i++) {
      const token = state.tokens[i]
      if (token.type === 'fence' && token.info.trim() === 'mermaid') {
        token.type = 'mermaid_diagram'
        token.tag = ''
        token.nesting = 0
      }
    }
    return true
  })

  md.renderer.rules.mermaid_diagram = (tokens, idx) => {
    const raw = tokens[idx].content.trim()
    const encoded = encodeURIComponent(raw)
    // data-mermaid 供客户端解码后渲染成 SVG。
    // visually-hidden 的 span 让本地搜索(minisearch)能索引到图表里的词
    // （节点标签、类名、时序消息等）；否则占位 div 文本为空，图表内的词搜不到。
    return (
      `<div class="mermaid-diagram" data-mermaid="${encoded}" data-rendered="false">` +
      `<span class="mermaid-search-text" aria-hidden="true">${escapeHtml(raw)}</span>` +
      `</div>`
    )
  }
}
