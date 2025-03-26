<script lang="ts">
    import { Collapse } from '@sveltestrap/sveltestrap';
  
    export let title: string;
    export let content: string;
    // Default to open; can be controlled via props if needed.
    let isOpen: boolean = true;
  </script>
  
  <div class="unit">
    <h2 on:click={() => isOpen = !isOpen} class="unit-header">
      {title} <span class="caret">{isOpen ? '▼' : '▶'}</span>
    </h2>
    <Collapse isOpen={isOpen}>
      <div class="unit-content">
        {@html content}
      </div>
    </Collapse>
  </div>
  
  <style>
    .unit {
      margin-bottom: 1rem;
      padding: 0.5rem;
      border-bottom: 1px solid #ccc;
    }
    .unit-header {
      cursor: pointer;
      user-select: none;
      margin: 0;
    }
    .caret {
      margin-left: 0.5rem;
      font-size: 0.8em;
    }
    .unit-content {
      margin-top: 0.5rem;
    }
  </style>
  