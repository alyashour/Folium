<!-- ledger/src/routes/+layout.svelte -->
<script lang="ts">
  import { PUBLIC_LEDGER_PING_URL } from '$env/static/public';
  import FetchOverride from '$lib/components/FetchOverride.svelte';
  import Footer from '$lib/components/Footer.svelte';
  import Navbar from '$lib/components/Navbar.svelte';
  import Notification from '$lib/components/Notification.svelte';

  let globalError = false;

  // These functions are used by FetchOverride to set error state for any fetch failures.
  function handleFetchError() {
    globalError = true;
  }

  function handleFetchSuccess() {
    // Optionally, you can set globalError = false here if you want to auto-hide on a success.
    // For now, we won't override the user's manual retry action.
  }

  // Retry function that tries to ping the server immediately.
  async function retry() {
    globalError = false; // hide modal before retrying
    try {
      const res = await fetch(PUBLIC_LEDGER_PING_URL);
      if (!res.ok) {
        globalError = true; // if ping fails, show modal again
      } else {
        globalError = false; // connection is back up
      }
    } catch (error) {
      globalError = true; // if error during fetch, re-show modal
    }
  }
</script>

<div class="layout-container">
  <Navbar />

  <!-- This component overrides fetch on the client side -->
  <FetchOverride onError={handleFetchError} onSuccess={handleFetchSuccess} />

  {#if globalError}
    <Notification onRetry={retry} />
  {/if}

  <main class="site-main">
    <slot />
  </main>

  <Footer />
</div>

<style>
  .layout-container {
    display: flex;
    flex-direction: column;
    min-height: 100vh;
  }
  .site-main {
    flex: 1;
    margin: 20px 90px;
  }
</style>
