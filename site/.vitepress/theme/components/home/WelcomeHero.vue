<script setup lang="ts">
import { withBase } from 'vitepress'
import InteractiveDemo from './InteractiveDemo.vue'
</script>

<template>
  <section class="welcome-hero ide-wrap">
    <div class="welcome-hero__main">
      <p class="ide-eyebrow">WELCOME · AWESOMEQT WORKBENCH</p>
      <h1 class="welcome-hero__title">把 Qt 学进<br />一个 Qt 应用里</h1>
      <p class="welcome-hero__tag">
        聚焦 C++ / QtWidgets 的三层递进教程 + 可复用实例库——
        这个网站本身就长得像一个 Qt 桌面应用，点一点右边。
      </p>
      <div class="welcome-hero__actions">
        <a class="ide-btn ide-btn--primary" :href="withBase('/beginner/')">走主线</a>
        <a class="ide-btn" :href="withBase('/engineering/')">浏览实例库</a>
      </div>
    </div>

    <div class="ide-panel welcome-hero__panel">
      <div class="welcome-hero__panel-head">
        <span class="welcome-hero__panel-dot" aria-hidden="true" />
        <span>main.cpp · 交互演示</span>
        <span class="welcome-hero__panel-tag">signal → slot</span>
      </div>
      <div class="welcome-hero__panel-body">
        <InteractiveDemo />
      </div>
    </div>
  </section>
</template>

<style scoped>
.welcome-hero {
  display: grid;
  grid-template-columns: 1.1fr 0.9fr;
  gap: 48px;
  align-items: center;
  padding-top: 56px;
  padding-bottom: 48px;
}

.welcome-hero__title {
  margin: 14px 0 16px;
  font-size: clamp(2.4rem, 5vw, 3.7rem);
  font-weight: 700;
  line-height: 1.22;
  letter-spacing: 0.01em;
  color: var(--vp-c-text-1);
}

.welcome-hero__tag {
  margin: 0 0 28px;
  font-size: 1.02rem;
  line-height: 1.9;
  color: var(--vp-c-text-2);
}

.welcome-hero__actions {
  display: flex;
  gap: 12px;
  flex-wrap: wrap;
}

/* 演示面板：Creator 输出/属性面板的形态 */
.welcome-hero__panel-head {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 7px 14px;
  border-bottom: 1px solid var(--ide-border);
  background: var(--vp-c-bg-soft);
  font-family: var(--aq-mono);
  font-size: 11px;
  letter-spacing: 0.05em;
  color: var(--vp-c-text-3);
}

.welcome-hero__panel-dot {
  width: 8px;
  height: 8px;
  border-radius: 2px;
  background: var(--vp-c-brand-1);
}

.welcome-hero__panel-tag {
  margin-left: auto;
  padding: 1px 7px;
  border: 1px solid var(--ide-border);
  border-radius: 3px;
  font-size: 10px;
}

.welcome-hero__panel-body {
  padding: 6px 18px 18px;
}

@media (max-width: 959px) {
  .welcome-hero {
    grid-template-columns: 1fr;
    gap: 32px;
    padding-top: 40px;
  }

  .welcome-hero__panel {
    max-width: 360px;
    width: 100%;
    margin: 0 auto;
  }
}
</style>
