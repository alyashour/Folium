<script lang="ts">
  import { browser } from '$app/environment';
  import { page } from '$app/stores';
  import { onMount } from 'svelte';
// Tells us if we're in the browser
  import { goto } from '$app/navigation';
  import {
    Button,
    Collapse,
    Dropdown,
    DropdownItem,
    DropdownMenu,
    DropdownToggle,
    Input,
    Nav,
    Navbar,
    NavbarBrand,
    NavbarToggler,
    NavItem,
    NavLink
  } from '@sveltestrap/sveltestrap';

  let isOpen = false;
  let isLoggedIn = false;
  let username = "";

  // Step 1: use a function to update login state from sessionStorage
  function checkSessionStorage() {
    if (!browser) return; // Only run in the browser
    const token = sessionStorage.getItem("userToken");
    if (token) {
      isLoggedIn = true;
      username = sessionStorage.getItem("username") || "";
    } else {
      isLoggedIn = false;
      username = "";
    }
  }

  // Step 2: onMount ensures it runs only in the browser after SSR
  onMount(() => {
    checkSessionStorage();
  });

  // Step 3: Reactively re-check when the URL changes (i.e. new route)
  $: if ($page.url) {
    checkSessionStorage();
  }

  function handleUpdate(event: CustomEvent<boolean>) {
    isOpen = event.detail;
  }

  function signOut() {
    if (browser) {
      sessionStorage.removeItem("userToken");
      sessionStorage.removeItem("username");
    }
    isLoggedIn = false;
    goto("/log-in"); // Redirect to login page after sign-out
  }
</script>

<Navbar color="secondary-subtle" light expand="md" container="md">
  <NavbarBrand href="/">Folium</NavbarBrand>

  <!-- Search Bar in the center -->
  <form class="mx-auto">
    <Input type="text" placeholder="Search" class="mr-sm-2" />
  </form>

  <!-- Navbar Toggler for Mobile Views -->
  <NavbarToggler on:click={() => (isOpen = !isOpen)} />

  <!-- Collapsible Navbar -->
  <Collapse {isOpen} navbar expand="md" on:update={handleUpdate}>
    <Nav class="ms-auto" navbar>
      {#if isLoggedIn}
        <!-- Logged in: Show Profile + Sign Out -->
        <NavItem>
          <NavLink href="/profile">
            <Button color="primary">Profile</Button>
          </NavLink>
        </NavItem>
        <NavItem>
          <NavLink on:click={signOut} style="cursor: pointer;">
            <Button color="secondary">Sign Out</Button>
          </NavLink>
        </NavItem>
      {:else}
        <!-- Not logged in: Show Create Account + Log In -->
        <NavItem>
          <NavLink href="/create-account">
            <Button color="primary">Create Account</Button>
          </NavLink>
        </NavItem>
        <NavItem>
          <NavLink href="/log-in">
            <Button color="secondary">Log In</Button>
          </NavLink>
        </NavItem>
      {/if}
      <!-- Dropdown for More Options -->
      <Dropdown nav inNavbar>
        <DropdownToggle nav caret>More</DropdownToggle>
        <DropdownMenu end>
          <DropdownItem>Help</DropdownItem>
          <DropdownItem>About</DropdownItem>
          <DropdownItem>Contact</DropdownItem>
        </DropdownMenu>
      </Dropdown>
    </Nav>
  </Collapse>
</Navbar>

<style>
  .ms-auto {
    margin-left: auto;
  }
</style>
