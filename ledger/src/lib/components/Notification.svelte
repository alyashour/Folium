<script lang="ts">
  import { createEventDispatcher } from 'svelte';

  /**
   * Basic notification modal that displays a connection error
   * and includes a 'Retry' button. The parent listens to the 'retry' event.
   */
  export let message: string = "We weren't able to connect to the server.";

  const dispatch = createEventDispatcher();

  function handleRetry() {
    dispatch('retry');
  }
</script>

<div class="notification-backdrop">
  <div class="notification-modal">
    <h2 class="notification-title">Connection Error</h2>
    <p class="notification-message">{message}</p>
    <button class="notification-button" on:click={handleRetry}>Retry</button>
  </div>
</div>

<style>
/* Make sure the backdrop covers the entire screen */
.notification-backdrop {
  position: fixed;
  top: 0;
  left: 0;
  width: 100vw;
  height: 100vh;
  /* Slightly darker overlay */
  background-color: rgba(0, 0, 0, 0.6);
  /* Place it on top of everything */
  z-index: 9999;
  display: flex;
  align-items: center;
  justify-content: center;
}

/* The modal content itself */
.notification-modal {
  background: white;
  border-radius: 8px;
  padding: 1.5rem 2rem;
  max-width: 400px;
  width: 90%;
  box-shadow: 0 2px 10px rgba(0,0,0,0.2);
  text-align: center;
  z-index: 10000;
}

/* Title text styling */
.notification-title {
  margin-top: 0;
  margin-bottom: 1rem;
  font-size: 1.5rem;
}

/* Message styling */
.notification-message {
  margin-bottom: 1rem;
  font-size: 1rem;
}

/* Retry button styling */
.notification-button {
  display: inline-block;
  background: #007bff; /* a standard bootstrap-like blue */
  color: #fff;
  border: none;
  border-radius: 4px;
  padding: 0.5rem 1rem;
  cursor: pointer;
  font-size: 1rem;
}

/* Hover effect for the button */
.notification-button:hover {
  background: #0056b3;
}
</style>
