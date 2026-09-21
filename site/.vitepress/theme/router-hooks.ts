import { useRouter } from 'vitepress'

// VitePress 的 router.onBeforeRouteChange / onAfterRouteChange 都是单值属性，
// 不是事件订阅：任意组件再赋值就会覆盖前一个（tamcpp 曾因 ReadingProgress 与
// mermaid 各自赋值，SPA 跳转后 mermaid 不渲染；AwesomeQt 的 ReadingProgress
// 已改用 watch(route.path)，mermaid 是当前唯一赋值者——这里统一包一层订阅器
// 根治，新增路由切换逻辑一律走这里，别直接赋值。对齐 tamcpp router-hooks）。
type RouteHandler = (href: string) => void
type HookName = 'onBeforeRouteChange' | 'onAfterRouteChange'

const subscribers: Record<HookName, Set<RouteHandler>> = {
  onBeforeRouteChange: new Set(),
  onAfterRouteChange: new Set(),
}
const installed: Record<HookName, boolean> = {
  onBeforeRouteChange: false,
  onAfterRouteChange: false,
}

export function subscribeBeforeRouteChange(fn: RouteHandler): void {
  subscribe('onBeforeRouteChange', fn)
}

export function subscribeAfterRouteChange(fn: RouteHandler): void {
  subscribe('onAfterRouteChange', fn)
}

function subscribe(hook: HookName, fn: RouteHandler): void {
  subscribers[hook].add(fn)
  if (installed[hook]) return
  // 必须在 setup 上下文调用（mermaid 的 setupMermaid / 各组件 setup 均满足）。
  const router = useRouter()
  if (hook === 'onBeforeRouteChange') {
    router.onBeforeRouteChange = (href) => dispatch(hook, href)
  } else {
    router.onAfterRouteChange = (href) => dispatch(hook, href)
  }
  installed[hook] = true
}

function dispatch(hook: HookName, href: string): void {
  for (const fn of subscribers[hook]) {
    try {
      fn(href)
    } catch (e) {
      // 单个订阅者抛错不能连累其它订阅者（否则 mermaid 渲染失败会让别的钩子也不跑）。
      console.error(`[router-hook] ${hook} 订阅者抛错`, e)
    }
  }
}
