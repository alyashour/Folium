<script lang="ts">
    import { onMount } from 'svelte';
  
    // We'll receive two props:
    // 1) A function to set globalError to true
    // 2) A function to reset the error if we want
    export let onError: () => void;
    export let onSuccess: () => void;
  
    onMount(() => {
      // This code only runs in the browser, so 'window' is defined here.
      const originalFetch = window.fetch;
  
      window.fetch = async (...args) => {
        try {
          const response = await originalFetch(...args);
          // If the response isn't OK, we consider that an error
          if (!response.ok) {
            onError();
          } else {
            onSuccess();
          }
          return response;
        } catch (err) {
          // If fetch itself throws, also trigger the error state
          onError();
          throw err;
        }
      };
    });
  </script>
  
  <!-- 
    This component doesn’t render any visible UI; 
    it just overrides fetch in the browser once mounted. 
  -->
  