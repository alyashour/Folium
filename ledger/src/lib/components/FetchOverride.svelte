<script lang="ts">
  import { browser } from '$app/environment';
  import { createEventDispatcher, onMount } from 'svelte';

  /**
   * This component monkey-patches the global fetch function on the client side:
   *  - If a network error occurs (like `net::ERR_CONNECTION_REFUSED`), we dispatch an "error" event.
   *  - If we get a 5xx response, we also dispatch "error".
   *  - Otherwise (including 4xx responses), we dispatch "success".
   * 
   * The parent layout can listen with:
   *  <FetchOverride on:error={handleFetchError} on:success={handleFetchSuccess} />
   */
  const dispatch = createEventDispatcher();

  onMount(() => {
    if (!browser) return;

    // Preserve the original fetch
    const originalFetch = window.fetch;

    // Patch the global fetch
    window.fetch = async (input: RequestInfo | URL, init?: RequestInit) => {
      try {
        const response = await originalFetch(input, init);
        if (!response.ok && response.status >= 500) {
          // 5xx response => server error
          dispatch('error');
        } else {
          // 2xx or 4xx => we consider it a "success" in terms of connectivity
          dispatch('success');
        }
        return response;
      } catch (err) {
        // A thrown error typically means a network-level error e.g. net::ERR_CONNECTION_REFUSED
        dispatch('error');
        throw err; // Re-throw so calling code can handle it if needed
      }
    };
  });
</script>

<!-- This component doesn't render a UI; it's just for side-effects (fetch override) -->
<div style="display: none;"></div>
