<script setup lang="ts">
</script>

<template>
  <div class="card-grid">
    <slot />
  </div>
</template>

<style scoped>
.card-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(280px, 1fr));
  gap: 12px;
  margin: 1.5em 0;
}

@media (max-width: 639px) {
  .card-grid {
    grid-template-columns: 1fr;
  }
}
</style>
