import { nextTick, onMounted, watch } from 'vue'
import { useData, useRouter } from 'vitepress'

declare global {
  interface Window {
    mermaid?: {
      initialize: (config: Record<string, unknown>) => void
      render: (id: string, text: string) => Promise<{ svg: string; bindFunctions?: (el: Element) => void }>
    }
    __mermaidLoadingPromise__?: Promise<void>
  }
}

// 沿用与 ModernCPP 一致的 CDN 运行时方案（站点本身需联网访问）。
// 注意：cdn.jsdelivr.net 自 2022.5 起在大陆被墙；若收到国内读者反馈图表加载失败，
// 可把此 URL 换成 npmmirror 镜像，或把 mermaid.min.js 放进 site/public/ 自托管。
const MERMAID_CDN = 'https://cdn.jsdelivr.net/npm/mermaid@10.9.6/dist/mermaid.min.js'

type MermaidTheme = 'default' | 'dark'

let currentTheme: MermaidTheme | null = null

function mermaidConfig(theme: MermaidTheme): Record<string, unknown> {
  return {
    startOnLoad: false,
    securityLevel: 'loose',
    theme,
    flowchart: {
      htmlLabels: true,
      useMaxWidth: false,
      wrap: true,
      nodeSpacing: 60,
      rankSpacing: 70,
      padding: 20,
    },
    themeVariables: {
      fontSize: '15px',
    },
  }
}

function escapeHtml(s: string): string {
  return s
    .replaceAll('&', '&amp;')
    .replaceAll('<', '&lt;')
    .replaceAll('>', '&gt;')
    .replaceAll('"', '&quot;')
    .replaceAll("'", '&#39;')
}

function loadMermaid(): Promise<void> {
  if (typeof window === 'undefined') return Promise.resolve()

  if (window.mermaid) return Promise.resolve()
  if (window.__mermaidLoadingPromise__) return window.__mermaidLoadingPromise__

  window.__mermaidLoadingPromise__ = new Promise<void>((resolve, reject) => {
    const existing = document.querySelector<HTMLScriptElement>('script[data-mermaid-runtime]')
    if (existing) {
      existing.addEventListener('load', () => resolve())
      existing.addEventListener('error', () => reject(new Error('Failed to load Mermaid')))
      return
    }

    const script = document.createElement('script')
    script.src = MERMAID_CDN
    script.async = true
    script.dataset.mermaidRuntime = 'true'
    script.onload = () => resolve()
    script.onerror = () => reject(new Error(`Failed to load Mermaid from ${MERMAID_CDN}`))
    document.head.appendChild(script)
  })

  return window.__mermaidLoadingPromise__
}

/** 按当前主题初始化 mermaid；主题变化时重新 initialize（initialize 本身不重绘已有 SVG）。 */
function ensureInitialized(theme: MermaidTheme): void {
  const mermaid = window.mermaid
  if (!mermaid || currentTheme === theme) return
  mermaid.initialize(mermaidConfig(theme))
  currentTheme = theme
}

async function renderMermaidDiagrams(theme: MermaidTheme): Promise<void> {
  if (typeof window === 'undefined') return

  try {
    await loadMermaid()
  } catch {
    // CDN 加载失败：把所有未渲染的图降级成源码兜底，而不是抛未捕获的 Promise rejection。
    document
      .querySelectorAll<HTMLElement>('.mermaid-diagram[data-rendered="false"]')
      .forEach((el) => {
        const raw = el.dataset.mermaid
        el.dataset.rendered = 'error'
        if (raw) el.innerHTML = `<pre class="mermaid-error">${escapeHtml(decodeURIComponent(raw))}</pre>`
      })
    return
  }

  await nextTick()
  await new Promise<void>((r) => requestAnimationFrame(() => r()))

  const mermaid = window.mermaid
  if (!mermaid) return

  ensureInitialized(theme)

  const nodes = Array.from(
    document.querySelectorAll<HTMLElement>('.mermaid-diagram[data-rendered="false"]')
  )

  for (let i = 0; i < nodes.length; i++) {
    const el = nodes[i]
    const raw = el.dataset.mermaid
    if (!raw) continue

    const source = decodeURIComponent(raw)
    const id = `mermaid-${Date.now()}-${i}-${Math.random().toString(36).slice(2, 8)}`

    try {
      const { svg } = await mermaid.render(id, source)
      el.innerHTML = svg
      el.dataset.rendered = 'true'
    } catch {
      el.dataset.rendered = 'error'
      el.innerHTML = `<pre class="mermaid-error">${escapeHtml(source)}</pre>`
    }
  }
}

export function setupMermaid(): void {
  const router = useRouter()
  const { isDark } = useData()

  const render = () => renderMermaidDiagrams(isDark.value ? 'dark' : 'default')

  onMounted(render)
  router.onAfterRouteChange = render

  // 切换深浅色时，已渲染的 SVG 不会自动变色——重置占位再按新主题重渲染。
  watch(isDark, (dark) => {
    document.querySelectorAll<HTMLElement>('.mermaid-diagram').forEach((el) => {
      el.innerHTML = ''
      el.dataset.rendered = 'false'
    })
    renderMermaidDiagrams(dark ? 'dark' : 'default')
  })
}
