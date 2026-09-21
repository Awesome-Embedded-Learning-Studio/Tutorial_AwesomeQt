<script setup lang="ts">
import { withBase } from 'vitepress'
import InteractiveDemo from './InteractiveDemo.vue'
</script>

<template>
  <section class="welcome-hero ide-wrap">
    <div class="welcome-hero__main">
      <p class="ide-eyebrow">C++ · QT 6 · WIDGETS</p>
      <h1 class="welcome-hero__title">AwesomeQt</h1>
      <p class="welcome-hero__lead">把 Qt 学进一个 Qt 应用里</p>
      <p class="welcome-hero__tag">
        从第一个 Widgets 窗口到 Qt 源码内部，三层递进教程与可复用实例库放在同一张工作台上。
      </p>
      <div class="welcome-hero__actions">
        <a class="ide-btn ide-btn--primary" :href="withBase('/beginner/')">开始学习 <span aria-hidden="true">→</span></a>
        <a class="ide-btn" :href="withBase('/engineering/')">实例库</a>
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
  grid-template-columns: minmax(0, 1.05fr) minmax(440px, 0.95fr);
  gap: 88px;
  align-items: center;
  min-height: 510px;
  padding-top: 54px;
  padding-bottom: 54px;
  border-bottom: 1px solid var(--vp-c-divider);
}

.welcome-hero__title {
  margin: 16px 0 6px;
  font-size: 64px;
  font-weight: 760;
  line-height: 1.05;
  letter-spacing: 0;
  color: var(--vp-c-text-1);
}

.welcome-hero__lead {
  margin: 0 0 24px;
  font-size: 26px;
  font-weight: 600;
  line-height: 1.45;
  color: var(--vp-c-text-1);
}

.welcome-hero__tag {
  max-width: 620px;
  margin: 0 0 30px;
  font-size: 16px;
  line-height: 1.85;
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
  padding: 9px 14px;
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
  border-radius: var(--aq-radius-sm);
  background: var(--vp-c-brand-1);
}

.welcome-hero__panel-tag {
  margin-left: auto;
  padding: 1px 7px;
  border: 1px solid var(--ide-border);
  border-radius: var(--aq-radius-sm);
  font-size: 10px;
}

.welcome-hero__panel-body {
  padding: 12px 20px 20px;
}

@media (max-width: 959px) {
  .welcome-hero {
    grid-template-columns: 1fr;
    min-height: 0;
    gap: 40px;
    padding-top: 40px;
  }

  .welcome-hero__panel {
    max-width: 560px;
    width: 100%;
    margin: 0;
  }
}

@media (max-width: 639px) {
  .welcome-hero {
    gap: 26px;
    padding-top: 30px;
    padding-bottom: 28px;
  }

  .welcome-hero__title {
    font-size: 46px;
  }

  .welcome-hero__lead {
    font-size: 21px;
  }

  .welcome-hero__panel-body {
    padding: 6px 14px 12px;
  }
}
</style>
