<script setup lang="ts">
import { onMounted, onBeforeUnmount, ref } from 'vue'

// 可拖拽抽屉侧栏宽度(仅左栏;右大纲栏 TOC 固定 256px,不再提供拖拽——
// 全视口抽屉化改版后右侧孤零零一条拖拽条被打回,功能低频直接砍掉)。
// 左栏 --vp-sidebar-width 由 VitePress 全链路消费(sidebar 自身宽度),改这一个变量即联动。
// handle 是 fixed 竖条、贴抽屉右缘:定位与拖动计算读 .VPSidebar 实际几何,仅抽屉
//   打开时显示(class 感知靠单独的 VPSidebar attribute observer,它与 .VPContent 平级)。
// 默认宽度不写死:按当前卷 sidebar 最宽条目文字实测 + 留白 + buffer 动态求值
//   (用户拖过并存了宽度 = 有明确偏好,保存值优先;双击重置即恢复自适应并清掉保存值)。
// 宽度持久化 localStorage;首屏防闪由 config head 内联脚本(hydration 前注入 272 近似值)负责。

interface Dim { min: number; max: number; def: number; key: string; cssVar: string }

const CONF: Dim = { min: 200, max: 480, def: 272, key: 'vp-sidebar-width', cssVar: '--vp-sidebar-width' }

const clamp = (v: number, min: number, max: number) => Math.min(max, Math.max(min, v))

/** 读用户保存的宽度;没有或非法返回 null */
function savedWidth(): number | null {
  try {
    const v = parseInt(localStorage.getItem(CONF.key) || '')
    return v >= CONF.min && v <= CONF.max ? v : null
  } catch {
    return null
  }
}

/**
 * 自适应默认宽度:实测当前 sidebar 最宽条目文字右缘(含嵌套缩进;text 带
 * 省略号截断,scrollWidth-clientWidth 补回被截掉的部分)+ 右侧留白 32 + buffer 14
 * (字体渲染差异/滚动条余量),夹在 [min,max]。测不到回退 272。
 * 换卷 sidebar 内容不同,路由变化后要重测。
 */
function measureLeftDefault(): number {
  const sb = document.querySelector('.VPSidebar') as HTMLElement | null
  if (!sb) return CONF.def
  const base = sb.getBoundingClientRect().left
  let maxRight = 0
  sb.querySelectorAll<HTMLElement>('.VPSidebarItem .text').forEach((t) => {
    const w = t.getBoundingClientRect().right + (t.scrollWidth - t.clientWidth) - base
    if (w > maxRight) maxRight = w
  })
  if (maxRight <= 0) return CONF.def
  return clamp(Math.ceil(maxRight + 32 + 14), CONF.min, CONF.max)
}

const leftHandle = ref<HTMLElement | null>(null)

const applyVar = (px: number) =>
  document.documentElement.style.setProperty(CONF.cssVar, px + 'px')
const persist = (px: number) => {
  try { localStorage.setItem(CONF.key, String(px)) } catch { /* 隐私模式 / 配额 */ }
}

let drag: {
  lastV: number
  handle: HTMLElement
  onMove: (e: MouseEvent) => void
  onUp: () => void
} | null = null

function startDrag(e: MouseEvent) {
  e.preventDefault()
  const handle = e.currentTarget as HTMLElement
  handle.classList.add('is-active')
  document.body.classList.add('rs-resizing')

  // 用「位移」而非「绝对坐标」算新宽度,与布局无关恒正确。
  const startX = e.clientX
  const startWidth =
    parseInt(getComputedStyle(document.documentElement).getPropertyValue(CONF.cssVar)) || CONF.def

  const onMove = (ev: MouseEvent) => {
    const v = clamp(Math.round(startWidth + (ev.clientX - startX)), CONF.min, CONF.max)
    if (drag) drag.lastV = v
    applyVar(v)
    if (drag?.handle) {
      drag.handle.style.left = ev.clientX + 'px' // handle 跟随鼠标(按下点在右缘,故 = 新右缘)
    }
  }
  const onUp = () => {
    if (drag) persist(drag.lastV)
    handle.classList.remove('is-active')
    document.body.classList.remove('rs-resizing')
    document.removeEventListener('mousemove', onMove)
    document.removeEventListener('mouseup', onUp)
    drag = null
    updateLeftPosition() // 拖动结束重新精确对齐(offsetLeft+offsetWidth)
  }
  drag = { lastV: startWidth, handle, onMove, onUp }
  document.addEventListener('mousemove', onMove)
  document.addEventListener('mouseup', onUp)
}

function reset() {
  // 双击重置 = 回到自适应默认,并清掉保存值 → 内容增长后继续自适应
  applyVar(measureLeftDefault())
  try { localStorage.removeItem(CONF.key) } catch { /* 隐私模式 */ }
  updateLeftPosition()
}

// 仅在抽屉打开时显示左 handle;收起时隐藏(侧栏在屏外,handle 不能悬在正文上)
function updateLeftVisibility() {
  if (!leftHandle.value) return
  const sb = document.querySelector('.VPSidebar')
  leftHandle.value.style.display =
    sb && sb.classList.contains('open') ? '' : 'none'
}

// 左 handle 精确定位:用 offsetLeft + offsetWidth(不含 transform),避开 sidebar 入场过渡
// (translateX(-100%)→0)对 getBoundingClientRect 的干扰 —— 首屏即读到最终右边缘,无需等动画结束。
function updateLeftPosition() {
  if (!leftHandle.value) return
  const sb = document.querySelector('.VPSidebar') as HTMLElement | null
  if (sb) leftHandle.value.style.left = (sb.offsetLeft + sb.offsetWidth) + 'px'
}

let observer: MutationObserver | null = null
let leftTimer = 0

// 抽屉开合是 .VPSidebar 上的 class 切换(open),而它与 .VPContent 平级——
// 下面的 VPContent observer 监听不到。单独给 .VPSidebar 挂 attribute 观察,
// 元素重建时(v-if 切换)由 VPContent 的 childList 回调重新绑定。
let sbObserver: MutationObserver | null = null
let watchedSb: Element | null = null
function watchSidebarElement() {
  const sb = document.querySelector('.VPSidebar')
  if (sb === watchedSb) return
  watchedSb = sb
  sbObserver?.disconnect()
  if (sb && 'MutationObserver' in window) {
    if (!sbObserver) sbObserver = new MutationObserver(onMutate)
    sbObserver.observe(sb, { attributes: true, attributeFilter: ['class'] })
  }
}

// 自适应默认宽度:rAF 去抖地重测(路由换卷 sidebar 重建时 MutationObserver 高频触发,
// 每次都量 ~200 个 text 节点会抖布局)。用户存过宽度就不测。
let measureQueued = false
function queueMeasureLeftDefault() {
  if (measureQueued || savedWidth() !== null) return
  measureQueued = true
  requestAnimationFrame(() => {
    measureQueued = false
    applyVar(measureLeftDefault())
    updateLeftPosition()
  })
}

const onMutate = () => {
  watchSidebarElement()
  updateLeftVisibility()
  updateLeftPosition()
  queueMeasureLeftDefault()
}

onMounted(() => {
  // 恢复已存宽度(防闪脚本已在首屏注入近似值);没存过 → 立即按内容实测自适应
  const v = savedWidth()
  if (v !== null) applyVar(v)
  queueMeasureLeftDefault()
  onMutate()
  // sidebar 打开有 transform 入场过渡(translateX(-100%)→0,约 0.25s),过渡中
  // getBoundingClientRect 偏左,导致 handle 初始错位(拖动后才贴合)。
  // 多时机补校准,确保首屏即贴合:下一帧 / 过渡结束后(~350ms)/ 页面 load 后。
  requestAnimationFrame(updateLeftPosition)
  leftTimer = window.setTimeout(updateLeftPosition, 350)
  window.addEventListener('resize', updateLeftPosition, { passive: true })
  if (document.readyState !== 'complete') {
    window.addEventListener('load', updateLeftPosition)
  }
  const root = document.querySelector('.VPContent') || document.body
  if ('MutationObserver' in window) {
    observer = new MutationObserver(onMutate)
    observer.observe(root, {
      childList: true,
      subtree: true,
      attributes: true,
      attributeFilter: ['class'],
    })
  }
})

onBeforeUnmount(() => {
  observer?.disconnect()
  observer = null
  sbObserver?.disconnect()
  sbObserver = null
  watchedSb = null
  window.clearTimeout(leftTimer)
  window.removeEventListener('resize', updateLeftPosition)
  window.removeEventListener('load', updateLeftPosition)
  if (drag) {
    document.removeEventListener('mousemove', drag.onMove)
    document.removeEventListener('mouseup', drag.onUp)
    drag = null
  }
})
</script>

<template>
  <div
    ref="leftHandle"
    class="rs-handle rs-handle--left"
    role="separator"
    aria-orientation="vertical"
    aria-label="拖动调整侧栏宽度(双击重置)"
    @mousedown="startDrag($event)"
    @dblclick="reset"
  ></div>
</template>
