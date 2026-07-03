<script setup>
import { useData, withBase } from 'vitepress'

const { frontmatter } = useData()
</script>

<template>
  <section v-if="frontmatter.hero" class="qt-hero">
    <div class="qt-hero__inner">
      <div class="qt-hero__text">
        <p class="qt-hero__kicker">Qt 6 中文深度教程</p>
        <h1 class="qt-hero__name">{{ frontmatter.hero.name }}</h1>
        <p class="qt-hero__text-main">{{ frontmatter.hero.text }}</p>
        <p class="qt-hero__tagline">{{ frontmatter.hero.tagline }}</p>
        <div class="qt-hero__actions">
          <a
            v-for="action in frontmatter.hero.actions"
            :key="action.link"
            :href="withBase(action.link)"
            :class="['qh-btn', `qh-btn--${action.theme}`]"
          >
            {{ action.text }}
          </a>
        </div>
      </div>

      <div class="qt-hero__art" aria-hidden="true">
        <svg viewBox="0 0 440 360" xmlns="http://www.w3.org/2000/svg" role="img">
          <!-- 对象树分叉（从中央圆向外） -->
          <g stroke="var(--vp-c-divider)" stroke-width="1.2" fill="none">
            <path d="M184 138 L120 86 L78 96" />
            <path d="M256 138 L320 86 L362 96" />
            <path d="M184 202 L120 254 L78 264" />
            <path d="M256 202 L320 254 L362 264" />
            <path d="M120 86 L96 60" />
            <path d="M320 86 L344 60" />
          </g>

          <!-- 末端对象节点（小方块） -->
          <g stroke="var(--vp-c-brand-1)" stroke-width="1.5" fill="var(--vp-c-bg)">
            <rect x="68" y="86" width="20" height="20" rx="3" />
            <rect x="352" y="86" width="20" height="20" rx="3" />
            <rect x="68" y="254" width="20" height="20" rx="3" />
            <rect x="352" y="254" width="20" height="20" rx="3" />
            <rect x="86" y="50" width="14" height="14" rx="2" />
            <rect x="340" y="50" width="14" height="14" rx="2" />
          </g>

          <!-- 信号槽弧线（节点回流） -->
          <path d="M78 96 Q160 40 220 122" stroke="var(--vp-c-brand-3)" stroke-width="1.2" fill="none" stroke-dasharray="3 4" />
          <path d="M362 264 Q280 320 220 218" stroke="var(--vp-c-brand-3)" stroke-width="1.2" fill="none" stroke-dasharray="3 4" />

          <!-- 中央：事件循环（脉冲） -->
          <circle cx="220" cy="170" r="52" fill="none" stroke="var(--vp-c-brand-1)" stroke-width="2">
            <animate attributeName="r" values="52;56;52" dur="2.6s" repeatCount="indefinite" />
            <animate attributeName="opacity" values="1;0.55;1" dur="2.6s" repeatCount="indefinite" />
          </circle>
          <circle cx="220" cy="170" r="36" fill="none" stroke="var(--vp-c-brand-1)" stroke-width="1" stroke-dasharray="3 4" opacity="0.7" />
          <circle cx="220" cy="170" r="4" fill="var(--vp-c-brand-1)" />
          <text x="220" y="174" text-anchor="middle" font-size="11" font-family="var(--vp-font-family)" fill="var(--vp-c-text-2)" letter-spacing="1">LOOP</text>

          <!-- 标注引线 + 标签 -->
          <g stroke="var(--vp-c-text-3)" stroke-width="0.8" fill="none">
            <path d="M272 170 L330 150 L360 150" />
            <path d="M168 170 L110 190 L80 190" />
            <path d="M220 222 L220 300 L260 300" />
          </g>
          <g fill="var(--vp-c-text-3)" font-size="11" font-family="var(--vp-font-family)">
            <circle cx="360" cy="150" r="1.8" />
            <text x="366" y="153">事件循环</text>
            <circle cx="80" cy="190" r="1.8" />
            <text x="76" y="193" text-anchor="end">对象树</text>
            <circle cx="260" cy="300" r="1.8" />
            <text x="266" y="303">信号与槽</text>
          </g>
        </svg>
      </div>
    </div>
  </section>
</template>

<style scoped>
.qt-hero {
  padding: 56px 24px 32px;
  max-width: 80rem;
  margin: 0 auto;
}

.qt-hero__inner {
  display: flex;
  align-items: center;
  gap: 48px;
  flex-wrap: wrap;
}

.qt-hero__text {
  flex: 1 1 420px;
  min-width: 0;
}

.qt-hero__kicker {
  font-size: 0.8rem;
  font-weight: 600;
  letter-spacing: 0.16em;
  color: var(--vp-c-brand-1);
  text-transform: uppercase;
  margin: 0 0 12px;
}

.qt-hero__name {
  font-size: 3.4rem;
  font-weight: 800;
  line-height: 1.1;
  letter-spacing: -0.02em;
  margin: 0 0 18px;
  background: linear-gradient(
    135deg,
    var(--vp-c-brand-1) 0%,
    var(--vp-c-brand-2) 50%,
    var(--vp-c-brand-3) 100%
  );
  -webkit-background-clip: text;
  background-clip: text;
  -webkit-text-fill-color: transparent;
}

.qt-hero__text-main {
  font-size: 1.5rem;
  font-weight: 700;
  color: var(--vp-c-text-1);
  margin: 0 0 10px;
  line-height: 1.35;
}

.qt-hero__tagline {
  font-size: 1.05rem;
  color: var(--vp-c-text-2);
  line-height: 1.7;
  margin: 0 0 28px;
}

.qt-hero__actions {
  display: flex;
  gap: 14px;
  flex-wrap: wrap;
}

.qh-btn {
  display: inline-flex;
  align-items: center;
  padding: 10px 22px;
  border-radius: 999px;
  font-size: 0.95rem;
  font-weight: 600;
  text-decoration: none !important;
  border: 1.5px solid transparent;
  transition: all 0.25s ease;
}

.qh-btn--brand {
  background: var(--vp-c-brand-1);
  color: var(--vp-c-bg-elv);
  border-color: var(--vp-c-brand-1);
}

.qh-btn--brand:hover {
  background: var(--vp-c-brand-2);
  border-color: var(--vp-c-brand-2);
  transform: translateY(-2px);
}

.qh-btn--alt {
  background: transparent;
  color: var(--vp-c-brand-1);
  border-color: var(--vp-c-brand-1);
}

.qh-btn--alt:hover {
  background: var(--vp-c-brand-soft);
  transform: translateY(-2px);
}

.qt-hero__art {
  flex: 0 1 380px;
  min-width: 280px;
}

.qt-hero__art svg {
  width: 100%;
  height: auto;
  display: block;
}

@media (max-width: 959px) {
  .qt-hero {
    padding: 40px 20px 24px;
  }
  .qt-hero__inner {
    flex-direction: column-reverse;
    gap: 24px;
  }
  .qt-hero__art {
    flex: 0 0 auto;
    max-width: 360px;
  }
  .qt-hero__name {
    font-size: 2.6rem;
  }
}

@media (prefers-reduced-motion: reduce) {
  .qt-hero__art svg animate {
    display: none;
  }
}
</style>

<style>
/* 隐藏默认 VPHero（由 QtHero 在 home-hero-before 接管） */
.VPHome .VPHero {
  display: none !important;
}
</style>
