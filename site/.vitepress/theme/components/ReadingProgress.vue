<script setup lang="ts">
import { ref, onMounted, onBeforeUnmount, watch } from 'vue'
import { useRouter } from 'vitepress'

// 顶部阅读进度条：滚动时按 scrollTop / (scrollHeight - clientHeight) 算百分比。
// brand 渐变 + 微光，固定在视口顶部。
//
// ⚠ 路由切换检测用 watch(router.route.path)，**不要**用 router.onAfterRouteChange——
// 那是单值回调属性（赋值覆盖，非事件订阅），mermaid 的 setupMermaid 也要用它；
// 这里若再赋值会覆盖掉 mermaid 的路由重渲染，导致 SPA 切页后图表卡骨架。
const progress = ref(0)
const router = useRouter()

function update() {
  const el = document.documentElement
  const scrollTop = el.scrollTop || document.body.scrollTop
  const scrollHeight = el.scrollHeight - el.clientHeight
  progress.value = scrollHeight > 0 ? Math.min(100, (scrollTop / scrollHeight) * 100) : 0
}

onMounted(() => {
  update()
  window.addEventListener('scroll', update, { passive: true })
  window.addEventListener('resize', update, { passive: true })
})

// 路由切换后重算（短页也要归零）
watch(() => router.route.path, () => {
  requestAnimationFrame(update)
  setTimeout(update, 300)
})

onBeforeUnmount(() => {
  window.removeEventListener('scroll', update)
  window.removeEventListener('resize', update)
})
</script>

<template>
  <div class="reading-progress" aria-hidden="true">
    <div class="reading-progress__bar" :style="{ width: progress + '%' }" />
  </div>
</template>

<style scoped>
.reading-progress {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  height: 3px;
  z-index: 100;
  pointer-events: none;
  background: transparent;
}

.reading-progress__bar {
  height: 100%;
  background: linear-gradient(90deg, var(--vp-c-brand-1), var(--vp-c-brand-3));
  box-shadow: 0 0 8px color-mix(in srgb, var(--vp-c-brand-1) 35%, transparent);
  transition: width 0.12s ease-out;
}

@media print {
  .reading-progress { display: none; }
}
</style>
