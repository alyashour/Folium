<script lang="ts">
    import { onMount } from "svelte";
    import { Container, Tooltip } from "@sveltestrap/sveltestrap";
    import { version, dev } from "$app/environment";
    import { PUBLIC_LEDGER_PING_URL } from "$env/static/public";

    let serverStatus: "up" | "error" | "timeout" | "unknown" = "unknown";

    $: statusColor =
        serverStatus === "up"
            ? "green"
            : serverStatus === "error"
              ? "red"
              : serverStatus === "timeout"
                ? "orange"
                : "yellow";

    $: statusText =
        serverStatus === "up"
            ? "Connected"
            : serverStatus === "error"
              ? "Not connected"
              : serverStatus === "timeout"
                ? "Timed out"
                : "Unknown";

    async function pingServer() {
        try {
            const controller = new AbortController();
            const timeout = setTimeout(() => controller.abort(), 5000);
            const response = await fetch(PUBLIC_LEDGER_PING_URL, {
                signal: controller.signal,
            });
            clearTimeout(timeout);
            serverStatus = response.ok ? "up" : "error";
        } catch (error: any) {
            serverStatus = error.name === "AbortError" ? "timeout" : "error";
        }
    }

    onMount(() => {
        pingServer();
        const interval = setInterval(pingServer, 10 * 60 * 1000); // ping every 10 mins
        return () => clearInterval(interval);
    });
</script>

<footer class="footer bg-dark text-white py-4">
    <Container>
        <p><i>Folium</i> © 2025</p>
        <a href="https://github.com/alyashour/Ledger">Github</a>
        <p>Using Ledger v{version}{dev ? "-dev" : ""}</p>
        <p>
            Made by Aly Ashour, Hamza Elkababji, Saad Elbayaty, and Omar Ramadan
        </p>
    </Container>
    <!-- Indicator Light -->
    <div
        id="ledger-indicator"
        class="indicator"
        style="background-color: {statusColor};"
    ></div>
    <!-- Tooltip tied to the indicator -->
    <Tooltip target="ledger-indicator" placement="top">
        Server status: {statusText}
    </Tooltip>
</footer>

<style>
    footer {
        position: relative; /* So the indicator positions relative to the footer */
        background-color: #333;
        color: white;
    }
    .indicator {
        position: absolute;
        bottom: 10px;
        right: 10px;
        width: 15px;
        height: 15px;
        border-radius: 50%;
        border: 1px solid #fff; /* Optional border for better visibility */
    }
    :global(body) {
        font-family: sans-serif;
    }
</style>
