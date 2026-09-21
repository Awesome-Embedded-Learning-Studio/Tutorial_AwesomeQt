import { nextTick, onMounted, watch } from 'vue'
import { useData } from 'vitepress'
import { openMermaidLightbox } from './mermaid-lightbox'
import { subscribeAfterRouteChange } from './router-hooks'

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
      attachMaximize(el, source)
    } catch (e) {
      el.dataset.rendered = 'error'
      el.innerHTML = `<pre class="mermaid-error">${escapeHtml(source)}</pre>`
      console.error('[mermaid] render failed for:\n' + source, e)
    }
  }
}

// ── maximize 按钮:每张图都挂(跟 GitHub 一样,所有图都可缩放),点开进全屏模态 ──
// 样式见 custom.css 的 .mermaid-maximize-btn / .mermaid-lightbox 段（对齐 tamcpp）。

// Feather maximize-2 图标(四角向外箭头),currentColor 随主题。
const MAXIMIZE_ICON =
  '<svg viewBox="0 0 24 24" width="15" height="15" fill="none" stroke="currentColor" ' +
  'stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">' +
  '<polyline points="15 3 21 3 21 9"/><polyline points="9 21 3 21 3 15"/>' +
  '<line x1="21" y1="3" x2="14" y2="10"/><line x1="3" y1="21" x2="10" y2="14"/></svg>'

function attachMaximize(el: HTMLElement, source: string) {
  const svg = el.querySelector('svg')
  if (!svg) return
  el.classList.add('mermaid-diagram--zoomable')

  const btn = document.createElement('button')
  btn.type = 'button'
  btn.className = 'mermaid-maximize-btn'
  btn.setAttribute('aria-label', '放大查看图表')
  btn.title = '放大查看图表'
  btn.innerHTML = MAXIMIZE_ICON
  btn.addEventListener('click', () => {
    openMermaidLightbox({ svg, source, trigger: btn })
  })
  el.appendChild(btn)
}

export function setupMermaid(): void {
  const { isDark } = useData()

  const render = () =>
    renderMermaidDiagrams(isDark.value ? 'dark' : 'default').catch((e) =>
      console.error('[mermaid] 渲染失败', e),
    )

  // 用订阅器而非直接赋值 router.onAfterRouteChange:后者是单值属性,
  // 会被后挂的组件覆盖,导致 SPA 跳转后 mermaid 不渲染(见 router-hooks.ts 注释)。
  // .catch 治「静默失败」:之前 onMounted 调用没接住 reject,加载失败时图直接消失无痕。
  onMounted(render)
  subscribeAfterRouteChange(render)

  // 切换深浅色时，已渲染的 SVG 不会自动变色——重置占位再按新主题重渲染。
  watch(isDark, (dark) => {
    document.querySelectorAll<HTMLElement>('.mermaid-diagram').forEach((el) => {
      el.innerHTML = ''
      el.dataset.rendered = 'false'
    })
    renderMermaidDiagrams(dark ? 'dark' : 'default')
  })
}
