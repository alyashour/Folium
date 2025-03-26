<script lang="ts">
    import { onMount } from "svelte";
    import { Container, Tooltip } from "@sveltestrap/sveltestrap";
    import { version, dev } from "$app/environment";
    import { PUBLIC_LEDGER_PING_URL } from "$env/static/public";

    let ledgerStatus: "up" | "error" | "timeout" | "unknown" = "unknown";

    $: statusColor =
        ledgerStatus === "up"
            ? "green"
            : ledgerStatus === "error"
              ? "red"
              : ledgerStatus === "timeout"
                ? "orange"
                : "yellow";

    $: statusText =
        ledgerStatus === "up"
            ? "Connected"
            : ledgerStatus === "error"
              ? "Not connected"
              : ledgerStatus === "timeout"
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
            ledgerStatus = response.ok ? "up" : "error";
        } catch (error: any) {
            ledgerStatus = error.name === "AbortError" ? "timeout" : "error";
        }
    }

    onMount(() => {
        pingServer();
        const interval = setInterval(pingServer, 10000);
        return () => clearInterval(interval);
    });
</script>

<footer class="footer bg-dark text-white py-4">
    <Container>
        <p><i>Folium</i> © 2025</p>
        <a href="https://www.github.com">Github</a>
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
