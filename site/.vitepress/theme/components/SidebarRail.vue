<script setup lang="ts">
import { useSidebar } from 'vitepress/theme'

// 左缘窄条把手：全视口抽屉侧栏（issue #20）的桌面/平板开关。
// 点击转发给 VPLocalNav 的「菜单」按钮而不是直接 useSidebar().toggle()——
// dev 预打包会把 'vitepress/theme' 单独 optimize，其内部 composables/sidebar
// 与 Layout 所用分裂成两个实例，toggle 改不动真状态（线上实测孤儿 ref）。
// 真按钮 emit open-menu 由 Layout 处理，Esc/路由切换/遮罩点击关闭全部原生继承；
// ≥1280 时按钮虽 display:none，JS .click() 仍触发完整事件链。
// 开合状态无需跟踪：抽屉 z-index 60 盖住窄条（30），关掉自然露出。
const { hasSidebar } = useSidebar()

function toggle() {
  document.querySelector<HTMLButtonElement>('.VPLocalNav .menu')?.click()
}
</script>

<template>
  <button
    v-if="hasSidebar"
    class="sidebar-rail"
    aria-controls="VPSidebarNav"
    aria-label="展开教程目录"
    @click="toggle"
  >
    <span class="sidebar-rail__icon"></span>
    <span class="sidebar-rail__label">目录</span>
  </button>
</template>

<style scoped>
.sidebar-rail {
  position: fixed;
  top: 50%;
  left: 0;
  transform: translateY(-50%);
  z-index: var(--vp-z-index-nav); /* 低于遮罩(50)/抽屉(60)，抽屉打开时自然被盖住 */
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  gap: 10px;
  width: 32px;
  height: 128px;
  border: 1px solid var(--vp-c-border);
  border-left: none;
  border-radius: 0 12px 12px 0;
  background: var(--vp-c-bg-elv);
  box-shadow: 2px 0 10px rgba(0, 0, 0, 0.1);
  color: var(--vp-c-text-2);
  cursor: pointer;
  opacity: 0.9;
  transition: color 0.2s ease, border-color 0.2s ease, box-shadow 0.2s ease,
    width 0.2s ease;
}

.sidebar-rail:hover {
  color: var(--vp-c-brand-1);
  border-color: var(--vp-c-brand-1);
  box-shadow: 2px 0 14px var(--vp-c-brand-soft);
  width: 36px;
}

/* 自绘箭头（vpi 图标字号上限太保守，把手要大要明显就用 SVG 级尺寸） */
.sidebar-rail__icon {
  width: 0;
  height: 0;
  border-top: 7px solid transparent;
  border-bottom: 7px solid transparent;
  border-left: 11px solid currentColor;
  transition: transform 0.2s ease;
}

.sidebar-rail:hover .sidebar-rail__icon {
  transform: translateX(2px);
}

.sidebar-rail__label {
  writing-mode: vertical-rl;
  font-size: 12px;
  letter-spacing: 0.35em;
  font-weight: 600;
}

/* <960px 有 VPLocalNav 菜单按钮，窄条不登场 */
@media (max-width: 959.98px) {
  .sidebar-rail {
    display: none;
  }
}
</style>
