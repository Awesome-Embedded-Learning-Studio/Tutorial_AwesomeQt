import type { PluginSimple } from 'markdown-it'
import type MarkdownIt from 'markdown-it'

/**
 * 快捷键渲染：识别 ++...++ 包裹的按键组合，渲染成连写的 <kbd> 标签。
 *   ++Ctrl+S++       → <kbd>Ctrl</kbd>+<kbd>S</kbd>
 *   ++Ctrl+Shift+P++ → <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>P</kbd>
 *
 * 用 html_inline 版（token.content = parts.map(kbd).join('+')），不用会渲染成单 kbd 含字面 + 的 token-push 版。
 * inline ruler after emphasis；拒绝前导字母数字/下划线，避开 C++ / i++ / operator++ / foo_++。
 */
export const kbdPlugin: PluginSimple = (md: MarkdownIt) => {
  md.inline.ruler.after('emphasis', 'kbd', (state, silent) => {
    const start = state.pos
    const max = state.posMax

    if (state.src.charCodeAt(start) !== 0x2B /* + */) return false
    if (state.src.charCodeAt(start + 1) !== 0x2B /* + */) return false

    // 前导字符是字母数字/下划线则拒绝（C++ / i++ / operator++ / foo_++）
    if (start > 0) {
      const prev = state.src.charCodeAt(start - 1)
      if (
        (prev >= 0x30 && prev <= 0x39) || // 0-9
        (prev >= 0x41 && prev <= 0x5A) || // A-Z
        (prev >= 0x61 && prev <= 0x7A) || // a-z
        prev === 0x5F // _
      ) return false
    }

    let pos = start + 2
    while (pos < max) {
      if (state.src.charCodeAt(pos) === 0x2B /* + */ && state.src.charCodeAt(pos + 1) === 0x2B /* + */) {
        const content = state.src.slice(start + 2, pos)
        if (content.length === 0) return false
        if (!silent) {
          const parts = content.split('+')
          const token = state.push('html_inline', '', 0)
          token.content = parts.map(p => `<kbd>${p.trim()}</kbd>`).join('+')
        }
        state.pos = pos + 2
        return true
      }
      pos++
    }
    return false
  })
}
