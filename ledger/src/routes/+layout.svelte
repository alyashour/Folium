<script lang="ts">
    import Footer from '$lib/components/Footer.svelte';
    import Navbar from '$lib/components/Navbar.svelte';
    import Notification from '$lib/components/Notification.svelte';
  </script>
  
  <div class="layout-container">
    <Navbar />
    <Notification />
  
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
