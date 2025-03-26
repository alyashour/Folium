<script lang="ts">
    import Navbar from "$lib/components/Navbar.svelte";
    import Footer from "$lib/components/Footer.svelte";
    let { children } = $props();
</script>

<div class="layout-container">
    <Navbar />

    <main class="site-main">
        {@render children()}
    </main>

    <Footer />
</div>

<style>
    .layout-container {
        display: flex;
        flex-direction: column;
        min-height: 100vh;
    }

    /* Let main content grow to fill the available space */
    .site-main {
        flex: 1;
        margin: 20px 90px;
    }
</style>
