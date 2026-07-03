<script setup>
// 篇首横幅：左侧篇号标记（默认 PART，可经 label 覆盖）+ 标题 + 副标题 + 计数。
// motif 选 theory（同心圆）/ specimen（带斜线的矩形）/ practice（分支树）三种抽象图形。
defineProps({
  act: { type: String, required: true },
  title: { type: String, required: true },
  subtitle: { type: String, default: '' },
  motif: { type: String, default: 'theory' },
  count: { type: String, default: '' },
  label: { type: String, default: 'PART' },
})
</script>

<template>
  <div class="chapter-hero" :class="`motif-${motif}`">
    <div class="chapter-hero__inner">
      <div class="chapter-hero__mark">
        <span class="chapter-hero__act-label">{{ label }}</span>
        <span class="chapter-hero__act-num">{{ act }}</span>
        <svg v-if="motif === 'theory'" class="chapter-hero__motif" viewBox="0 0 40 40" aria-hidden="true">
          <circle cx="20" cy="20" r="14" fill="none" stroke="var(--vp-c-brand-1)" stroke-width="1.2" opacity="0.45" />
          <circle cx="20" cy="20" r="8" fill="none" stroke="var(--vp-c-brand-1)" stroke-width="1.2" opacity="0.7" />
          <circle cx="20" cy="20" r="2" fill="var(--vp-c-brand-1)" />
          <g fill="var(--vp-c-text-3)" opacity="0.55">
            <circle cx="6" cy="6" r="1" /><circle cx="34" cy="6" r="1" />
            <circle cx="6" cy="34" r="1" /><circle cx="34" cy="34" r="1" />
          </g>
        </svg>
        <svg v-else-if="motif === 'specimen'" class="chapter-hero__motif" viewBox="0 0 40 40" aria-hidden="true">
          <rect x="6" y="8" width="22" height="24" rx="2" fill="none" stroke="var(--vp-c-brand-1)" stroke-width="1.2" />
          <line x1="4" y1="36" x2="36" y2="4" stroke="var(--vp-c-brand-1)" stroke-width="1.6" stroke-linecap="round" />
          <circle cx="34" cy="6" r="2.2" fill="var(--vp-c-brand-1)" />
        </svg>
        <svg v-else class="chapter-hero__motif" viewBox="0 0 40 40" aria-hidden="true">
          <path d="M20 6 L20 20 M20 20 L10 30 M20 20 L30 30 M10 30 L6 36 M10 30 L14 36 M30 30 L26 36 M30 30 L34 36"
            fill="none" stroke="var(--vp-c-brand-1)" stroke-width="1.3" stroke-linecap="round" />
          <circle cx="20" cy="6" r="2.5" fill="var(--vp-c-brand-1)" />
        </svg>
      </div>
      <div class="chapter-hero__body">
        <h2 class="chapter-hero__title">{{ title }}</h2>
        <p v-if="subtitle" class="chapter-hero__subtitle">{{ subtitle }}</p>
        <p v-if="count" class="chapter-hero__count">{{ count }}</p>
      </div>
    </div>
    <div class="chapter-hero__rule">
      <span class="chapter-hero__tick" />
    </div>
  </div>
</template>

<style scoped>
.chapter-hero {
  margin: 1rem 0 2.5rem;
  padding: 2rem 1.5rem 1.5rem;
  background: var(--vp-c-bg-soft);
  border-radius: 12px;
  border-left: 4px solid var(--vp-c-brand-1);
  position: relative;
}

.chapter-hero__inner {
  display: flex;
  align-items: center;
  gap: 2rem;
  flex-wrap: wrap;
}

.chapter-hero__mark {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 0.25rem;
  flex-shrink: 0;
  min-width: 80px;
}

.chapter-hero__act-label {
  font-size: 0.7rem;
  font-weight: 700;
  letter-spacing: 0.2em;
  color: var(--vp-c-text-3);
}

.chapter-hero__act-num {
  font-size: 2.6rem;
  font-weight: 800;
  line-height: 1;
  color: var(--vp-c-brand-1);
}

.chapter-hero__motif {
  width: 40px;
  height: 40px;
  margin-top: 0.5rem;
}

.chapter-hero__body {
  flex: 1;
  min-width: 200px;
}

.chapter-hero__title {
  font-size: 1.8rem;
  font-weight: 700;
  margin: 0 0 0.4rem;
  color: var(--vp-c-text-1);
  line-height: 1.25;
  border: none;
  padding: 0;
}

.chapter-hero__subtitle {
  font-size: 1rem;
  color: var(--vp-c-text-2);
  margin: 0 0 0.5rem;
  line-height: 1.6;
}

.chapter-hero__count {
  font-size: 0.85rem;
  font-weight: 600;
  color: var(--vp-c-brand-1);
  margin: 0;
  letter-spacing: 0.04em;
}

.chapter-hero__rule {
  margin-top: 1.5rem;
  height: 1px;
  background: var(--vp-c-divider);
  position: relative;
}

.chapter-hero__tick {
  position: absolute;
  left: 0;
  top: -3px;
  width: 7px;
  height: 7px;
  background: var(--vp-c-brand-1);
  transform: rotate(45deg);
}

@media (max-width: 639px) {
  .chapter-hero {
    padding: 1.5rem 1rem 1rem;
  }
  .chapter-hero__inner {
    gap: 1rem;
  }
  .chapter-hero__mark {
    min-width: 60px;
  }
  .chapter-hero__act-num {
    font-size: 2rem;
  }
  .chapter-hero__title {
    font-size: 1.4rem;
  }
}
</style>
