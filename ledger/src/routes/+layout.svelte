<!-- ledger/src/routes/+layout.svelte -->
<script lang="ts">
  import { PUBLIC_API_BASE_URL } from '$env/static/public';
  import FetchOverride from '$lib/components/FetchOverride.svelte';
  import Footer from '$lib/components/Footer.svelte';
  import Navbar from '$lib/components/Navbar.svelte';
  import Notification from '$lib/components/Notification.svelte';
  import { setContext } from 'svelte';
  import { writable } from 'svelte/store';

  // Create a writable store that tracks whether a connection error exists.
  const connectionError = writable(false);

  /**
   * showConnectionError sets the connection error state to true.
   * Components (e.g. login) can call this function (from context) if necessary.
   */
  function showConnectionError() {
    connectionError.set(true);
  }

  // Provide showConnectionError via context so that child components can trigger it.
  setContext('showConnectionError', showConnectionError);

  /**
   * handleFetchError is called when FetchOverride dispatches an "error" event.
   * This indicates a network failure or a 500-level server error.
   */
  function handleFetchError() {
    connectionError.set(true);
  }

  /**
   * handleFetchSuccess is called when FetchOverride dispatches a "success" event.
   * For our purposes we don’t auto-clear the error so that only an explicit retry will remove it.
   */
  function handleFetchSuccess() {
    // Optionally clear the error on success—but here we leave it as-is until the user retries.
  }

  /**
   * retry pings the server using PUBLIC_API_BASE_URL. If the ping is successful,
   * it clears the connection error.
   */
  async function retry() {
    connectionError.set(false);
    try {
      const res = await fetch(PUBLIC_API_BASE_URL + "/ping");
      if (!res.ok) {
        connectionError.set(true);
      } else {
        connectionError.set(false);
      }
    } catch (error) {
      connectionError.set(true);
    }
  }
</script>

<div class="layout-container">
  <Navbar />

  <!-- Mount the FetchOverride component so that its customFetch events bubble up -->
  <FetchOverride on:error={handleFetchError} on:success={handleFetchSuccess} />

  {#if $connectionError}
    <!-- Display the connection error notification with a retry callback -->
    <Notification on:retry={retry} />
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
