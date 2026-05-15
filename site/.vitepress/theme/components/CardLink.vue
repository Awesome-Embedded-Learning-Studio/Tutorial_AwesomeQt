<script setup lang="ts">
import { withBase } from 'vitepress'

defineProps<{
  href: string
  num?: string
  desc?: string
}>()
</script>

<template>
  <a :href="withBase(href)" class="card-link">
    <span v-if="num" class="card-num">{{ num }}</span>
    <div class="card-body">
      <span class="card-title"><slot /></span>
      <span v-if="desc" class="card-desc">{{ desc }}</span>
    </div>
  </a>
</template>

<style scoped>
.card-link {
  display: flex;
  align-items: flex-start;
  gap: 12px;
  padding: 16px 18px;
  border: 1px solid var(--vp-c-divider);
  border-radius: 12px;
  background-color: var(--vp-c-bg);
  box-shadow: 0 1px 3px rgba(0, 0, 0, 0.04),
              0 1px 2px rgba(0, 0, 0, 0.06);
  text-decoration: none !important;
  color: inherit;
  transition: border-color 0.3s ease,
              box-shadow 0.3s ease,
              transform 0.3s ease;
}

.card-link:hover {
  border-color: var(--vp-c-brand-1);
  box-shadow: 0 8px 24px rgba(0, 0, 0, 0.08),
              0 2px 6px rgba(0, 0, 0, 0.04);
  transform: translateY(-2px);
  text-decoration: none !important;
}

.dark .card-link {
  background-color: var(--vp-c-bg-elv);
  border-color: var(--vp-c-border);
}

.dark .card-link:hover {
  box-shadow: 0 8px 24px rgba(0, 0, 0, 0.25),
              0 2px 6px rgba(0, 0, 0, 0.15);
}

.card-num {
  flex-shrink: 0;
  font-size: 0.72rem;
  font-weight: 600;
  padding: 2px 8px;
  border-radius: 6px;
  background: var(--vp-c-brand-soft);
  color: var(--vp-c-brand-1);
  line-height: 1.6;
  margin-top: 1px;
}

.card-body {
  min-width: 0;
  display: flex;
  flex-direction: column;
  gap: 4px;
}

.card-title {
  font-size: 0.88rem;
  font-weight: 600;
  color: var(--vp-c-text-1);
  line-height: 1.5;
  transition: color 0.3s ease;
}

.card-link:hover .card-title {
  color: var(--vp-c-brand-1);
}

.card-desc {
  font-size: 0.78rem;
  color: var(--vp-c-text-3);
  line-height: 1.5;
}

@media (max-width: 639px) {
  .card-link {
    padding: 14px 16px;
  }
}
</style>
