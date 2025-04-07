<script lang="ts">
    import { notification } from '$lib/stores/notification';
    import { onDestroy } from 'svelte';
    
    let message = '';
    const unsubscribe = notification.subscribe((msg) => {
      message = msg;
      if (msg) {
        setTimeout(() => notification.set(''), 5000); // Auto-clear after 5 seconds
      }
    });
    
    onDestroy(() => unsubscribe());
  </script>
  
  {#if message}
    <div class="notification">
      {message}
    </div>
  {/if}
  
  <style>
    .notification {
      position: fixed;
      top: 1rem;
      right: 1rem;
      background-color: #ff4d4f;
      color: white;
      padding: 1rem;
      border-radius: 8px;
      z-index: 1000;
      box-shadow: 0 4px 6px rgba(0,0,0,0.1);
    }
  </style>
  