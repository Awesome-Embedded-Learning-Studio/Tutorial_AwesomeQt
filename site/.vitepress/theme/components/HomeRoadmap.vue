<script setup lang="ts">
import { withBase } from 'vitepress'

const layers = [
  {
    id: 'beginner',
    title: '入门层',
    status: 'done',
    statusText: '已完成',
    count: '137 篇 · 134 示例',
    desc: '从零开始，覆盖 QtBase、QtGui、QtWidgets、QtNetwork、QML 全部核心模块',
    link: '/beginner/',
  },
  {
    id: 'advanced',
    title: '进阶层',
    status: 'progress',
    statusText: '规划中',
    count: '预估 137 篇',
    desc: '高级用法、性能优化、工程实践',
    link: '/advanced/',
  },
  {
    id: 'expert',
    title: '专家层',
    status: 'progress',
    statusText: '规划中',
    count: '预估 142 篇',
    desc: 'MOC 原理、信号槽源码、内存模型、事件循环源码',
    link: '/expert/',
  },
]
</script>

<template>
  <div class="roadmap">
    <h2 class="roadmap-title">学习路线</h2>
    <div class="roadmap-track">
      <a
        v-for="(layer, i) in layers"
        :key="layer.id"
        :href="withBase(layer.link)"
        class="roadmap-node"
        :class="[layer.status, { last: i === layers.length - 1 }]"
      >
        <div class="node-indicator">
          <span v-if="layer.status === 'done'" class="node-check">&#10003;</span>
          <span v-else class="node-dot" />
        </div>
        <div v-if="i < layers.length - 1" class="node-line" />
        <div class="node-body">
          <div class="node-header">
            <span class="node-title">{{ layer.title }}</span>
            <span class="node-badge" :class="layer.status">{{ layer.statusText }}</span>
          </div>
          <span class="node-count">{{ layer.count }}</span>
          <p class="node-desc">{{ layer.desc }}</p>
        </div>
      </a>
    </div>
  </div>
</template>

<style scoped>
.roadmap {
  max-width: 720px;
  margin: 3rem auto 0;
  padding: 0 1.5rem;
}

.roadmap-title {
  text-align: center;
  font-size: 1.25rem;
  font-weight: 600;
  margin-bottom: 2rem;
  color: var(--vp-c-text-1);
}

.roadmap-track {
  display: flex;
  flex-direction: column;
}

.roadmap-node {
  display: flex;
  align-items: flex-start;
  gap: 1rem;
  position: relative;
  text-decoration: none;
  color: inherit;
  padding: 0.75rem 1rem;
  border-radius: 10px;
  transition: background 0.2s;
}

.roadmap-node:hover {
  background: var(--vp-c-default-soft);
}

.node-indicator {
  display: flex;
  flex-direction: column;
  align-items: center;
  flex-shrink: 0;
  width: 28px;
  position: relative;
  z-index: 1;
}

.node-check {
  width: 28px;
  height: 28px;
  border-radius: 50%;
  background: var(--vp-c-green-1);
  color: #fff;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 14px;
  font-weight: 700;
}

.node-dot {
  width: 14px;
  height: 14px;
  border-radius: 50%;
  border: 3px solid var(--vp-c-brand-1);
  background: var(--vp-c-bg);
  margin: 7px 0;
}

.node-line {
  position: absolute;
  left: 13px;
  top: 36px;
  bottom: -8px;
  width: 2px;
  background: var(--vp-c-divider);
}

.roadmap-node.last .node-line {
  display: none;
}

.node-body {
  flex: 1;
  min-width: 0;
}

.node-header {
  display: flex;
  align-items: center;
  gap: 0.5rem;
  margin-bottom: 0.15rem;
}

.node-title {
  font-size: 1.05rem;
  font-weight: 600;
  color: var(--vp-c-text-1);
}

.node-badge {
  font-size: 0.7rem;
  font-weight: 600;
  padding: 1px 8px;
  border-radius: 20px;
  line-height: 1.6;
  letter-spacing: 0.02em;
}

.node-badge.done {
  background: var(--vp-c-green-soft);
  color: var(--vp-c-green-1);
}

.node-badge.progress {
  background: var(--vp-c-brand-soft);
  color: var(--vp-c-brand-1);
}

.node-count {
  font-size: 0.8rem;
  color: var(--vp-c-text-3);
}

.node-desc {
  font-size: 0.85rem;
  color: var(--vp-c-text-2);
  margin: 0.2rem 0 0;
  line-height: 1.5;
}

@media (max-width: 639px) {
  .roadmap {
    padding: 0 0.75rem;
    margin-top: 2rem;
  }
  .roadmap-node {
    padding: 0.6rem 0.5rem;
  }
}
</style>
