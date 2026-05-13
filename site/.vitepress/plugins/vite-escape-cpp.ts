import type { Plugin } from 'vite'

const HTML_TAGS = new Set([
  'a','abbr','address','area','article','aside','audio',
  'b','base','bdi','bdo','blockquote','body','br','button',
  'canvas','caption','cite','code','col','colgroup',
  'data','datalist','dd','del','details','dfn','dialog','div','dl','dt',
  'em','embed','fieldset','figcaption','figure','footer','form',
  'h1','h2','h3','h4','h5','h6','head','header','hgroup','hr','html',
  'i','iframe','img','input','ins','kbd','label','legend','li','link',
  'main','map','mark','menu','meta','meter','nav','noscript',
  'object','ol','optgroup','option','output','p','picture','pre','progress',
  'q','rp','rt','ruby','s','samp','script','section','select','slot',
  'small','source','span','strong','style','sub','summary','sup',
  'table','tbody','td','template','textarea','tfoot','th','thead',
  'time','title','tr','track','u','ul','var','video','wbr',
  'client-only','content','doc-footer','doc-sidebar',
  'vp-code-group','vp-tab',
  'mjx-container','mjx-assistive-mml','mjx-body','mjx-math',
  'svg','path','g','rect','circle','line','polygon','polyline','text',
  'use','defs','clippath','lineargradient','radialgradient','stop',
  'title','desc','image','pattern','mask','marker','symbol','foreignobject',
])

function looksLikeCppTag(inner: string): boolean {
  const trimmed = inner.trim()
  if (!trimmed) return false
  if (HTML_TAGS.has(trimmed.toLowerCase())) return false
  // Covers: QWidget, QLabel, QPdfDocument, std::vector, int, char16_t, etc.
  return /^[A-Za-z_][A-Za-z0-9_:,\s*&.<>]*(?:\.\.\.)?$/.test(trimmed)
}

function escapeLine(line: string): string {
  const segments = line.split(/(``[^`]*``|`[^`]*`)/)
  return segments.map((seg, i) => {
    if (i % 2 === 1) return seg
    return seg.replace(/<([^<>\n]+)>/g, (match, inner: string) => {
      return looksLikeCppTag(inner)
        ? `&lt;${inner.trim()}&gt;`
        : match
    })
  }).join('')
}

export function viteCppEscape(): Plugin {
  return {
    name: 'vite-cpp-escape',
    enforce: 'pre',
    transform(code, id) {
      if (!id.endsWith('.md')) return null
      const lines = code.split('\n')
      let inFence = false
      let fenceChar = ''
      let changed = false

      const result = lines.map(line => {
        const fenceMatch = line.match(/^(\s*)(```+|~~~+)/)
        if (fenceMatch) {
          const marker = fenceMatch[2]
          if (!inFence) {
            inFence = true
            fenceChar = marker[0]
            return line
          }
          if (marker[0] === fenceChar && marker.length >= 3) {
            inFence = false
            fenceChar = ''
          }
          return line
        }

        if (inFence) return line

        const escaped = escapeLine(line)
        if (escaped !== line) changed = true
        return escaped
      })

      if (!changed) return null
      return { code: result.join('\n'), map: null }
    },
  }
}
