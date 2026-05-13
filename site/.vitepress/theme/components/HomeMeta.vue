<script setup lang="ts">
const items = [
  {
    icon: '🎯',
    title: '适合谁',
    lines: ['C++ 初学者 → 第一行代码到完整应用', '其他 GUI 经验 → 信号槽、对象树、元对象系统', 'Qt 5 老用户 → Qt 6 新变化 + CMake + 现代 C++', '工程项目 → 最佳实践、性能优化、跨平台部署'],
  },
  {
    icon: '⚙️',
    title: '技术栈',
    lines: ['Qt 6.9.1 · CMake 3.26+ · C++17/20', 'Windows / Linux / WSL2', 'MSVC / MinGW / GCC 11+'],
  },
  {
    icon: '✍️',
    title: '写作风格',
    lines: ['第一人称叙事，实战导向', '踩坑必写后果，禁止空话', '教程入门 → 官方文档深入 → 源码验证'],
  },
  {
    icon: '🤝',
    title: '贡献',
    lines: ['MIT 协议 · Issue / PR 欢迎', '每篇末尾官方文档链接均已验证'],
  },
]
</script>

<template>
  <div class="meta-grid">
    <div v-for="item in items" :key="item.title" class="meta-card">
      <div class="meta-icon">{{ item.icon }}</div>
      <div class="meta-body">
        <h3 class="meta-title">{{ item.title }}</h3>
        <ul class="meta-lines">
          <li v-for="line in item.lines" :key="line">{{ line }}</li>
        </ul>
      </div>
    </div>
  </div>
</template>

<style scoped>
.meta-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 16px;
  max-width: 720px;
  margin: 2.5rem auto 0;
  padding: 0 1.5rem;
}

.meta-card {
  display: flex;
  gap: 12px;
  padding: 18px 16px;
  border: 1px solid var(--vp-c-divider);
  border-radius: 12px;
  background: var(--vp-c-bg);
  transition: border-color 0.2s, box-shadow 0.2s;
}

.meta-card:hover {
  border-color: var(--vp-c-brand-1);
  box-shadow: 0 4px 16px rgba(0, 0, 0, 0.06);
}

.meta-icon {
  font-size: 1.4rem;
  flex-shrink: 0;
  line-height: 1;
  margin-top: 2px;
}

.meta-title {
  font-size: 0.9rem;
  font-weight: 600;
  margin: 0 0 6px;
  color: var(--vp-c-text-1);
}

.meta-lines {
  list-style: none;
  padding: 0;
  margin: 0;
}

.meta-lines li {
  font-size: 0.8rem;
  color: var(--vp-c-text-2);
  line-height: 1.6;
}

.meta-lines li::before {
  content: '·';
  margin-right: 6px;
  color: var(--vp-c-text-3);
}

@media (max-width: 639px) {
  .meta-grid {
    grid-template-columns: 1fr;
    padding: 0 0.75rem;
  }
}
</style>
