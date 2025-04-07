<!-- src/lib/components/Notification.svelte -->
<script lang="ts">
  export let onRetry: () => void;
</script>

<div class="modal-overlay">
  <div class="modal-content">
    <h2>Connection Error</h2>
    <p>We weren't able to connect to the server.</p>
    <button on:click={onRetry}>Retry</button>
  </div>
</div>

<style>
  .modal-overlay {
    position: fixed;
    top: 0;
    left: 0;
    right: 0;
    bottom: 0;
    background: rgba(0, 0, 0, 0.7);
    z-index: 1000;
    display: flex;
    justify-content: center;
    align-items: center;
  }
  .modal-content {
    background: #fff;
    padding: 2rem;
    border-radius: 8px;
    text-align: center;
    max-width: 400px;
    width: 90%;
  }
  button {
    margin-top: 1rem;
    padding: 0.75rem 1.5rem;
    border: none;
    background: #1890ff;
    color: white;
    border-radius: 4px;
    cursor: pointer;
  }
</style>
