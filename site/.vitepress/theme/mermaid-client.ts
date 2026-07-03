import { nextTick, onMounted, watch } from 'vue'
import { useData, useRouter } from 'vitepress'

// 本地打包 mermaid：dynamic import 客户端懒加载，SSR 不引入。Vite 自动 code-split 成独立 chunk。
// 不走 CDN —— 浏览器加载跨域 CDN 脚本不可靠（onload 永不触发会卡骨架）。

type MermaidTheme = 'default' | 'dark'

let mermaidApi: any = null
let currentTheme: MermaidTheme | null = null

async function ensureMermaid(): Promise<any> {
  if (mermaidApi) return mermaidApi
  mermaidApi = (await import('mermaid')).default
  return mermaidApi
}

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

/** 按当前主题初始化 mermaid；主题变化时重新 initialize（initialize 本身不重绘已有 SVG）。 */
async function ensureInitialized(theme: MermaidTheme): Promise<void> {
  const m = await ensureMermaid()
  if (currentTheme === theme) return
  m.initialize(mermaidConfig(theme))
  currentTheme = theme
}

function escapeHtml(s: string): string {
  return s
    .replaceAll('&', '&amp;')
    .replaceAll('<', '&lt;')
    .replaceAll('>', '&gt;')
    .replaceAll('"', '&quot;')
    .replaceAll("'", '&#39;')
}

async function renderMermaidDiagrams(theme: MermaidTheme): Promise<void> {
  if (typeof window === 'undefined') return

  try {
    await ensureInitialized(theme)
  } catch (e) {
    console.error('[mermaid] init failed', e)
    return
  }

  await nextTick()

  // SPA 路由切换：VitePress 的 onAfterRouteChange 在 loadPage（设 route.component）之后立即触发，
  // 但 Vue 把新页 component 渲染进 DOM 是异步的——触发那一刻新页的 mermaid 占位 div 还没挂载，
  // 一次 nextTick 等不到。轮询等节点出现，最多 ~1.5s；直载（onMounted）时内容已在 DOM，首次即命中。
  let nodes: HTMLElement[] = []
  for (let attempt = 0; attempt < 15; attempt++) {
    nodes = Array.from(
      document.querySelectorAll<HTMLElement>('.mermaid-diagram[data-rendered="false"]')
    )
    if (nodes.length > 0) break
    await new Promise<void>((r) => setTimeout(r, 100))
  }

  for (let i = 0; i < nodes.length; i++) {
    const el = nodes[i]
    const raw = el.dataset.mermaid
    if (!raw) continue

    const source = decodeURIComponent(raw)
    const id = `mermaid-${Date.now()}-${i}-${Math.random().toString(36).slice(2, 8)}`

    try {
      const m = await ensureMermaid()
      const { svg } = await m.render(id, source)
      el.innerHTML = svg
      el.dataset.rendered = 'true'
    } catch (e) {
      el.dataset.rendered = 'error'
      el.innerHTML = `<pre class="mermaid-error">${escapeHtml(source)}</pre>`
      console.error('[mermaid] render failed for:\n' + source, e)
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
