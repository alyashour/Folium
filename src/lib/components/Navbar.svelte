<script lang="ts">
    import {
      Collapse,
      NavbarToggler,
      NavbarBrand,
      Nav,
      Navbar,
      NavItem,
      NavLink,
      Dropdown,
      DropdownToggle,
      DropdownMenu,
      DropdownItem,
      Input,
      Button
    } from '@sveltestrap/sveltestrap';
  
    let isOpen = false;
  
    function handleUpdate(event: CustomEvent<boolean>) {
      isOpen = event.detail;
    }
  </script>
  
  <Navbar color="secondary-subtle" light expand="md" container="md">
    <NavbarBrand href="/">Folium</NavbarBrand>
  
    <!-- Search Bar using Input component -->
    <form class="mx-auto">
      <Input type="text" placeholder="Search" class="mr-sm-2" />
    </form>
  
    <!-- Navbar Toggler for Mobile Views -->
    <NavbarToggler on:click={() => (isOpen = !isOpen)} />
    
    <!-- Collapse Navbar on Mobile -->
    <Collapse {isOpen} navbar expand="md" on:update={handleUpdate}>
      <Nav class="ms-auto" navbar>
        <!-- Create Account Button -->
        <NavItem>
          <NavLink href="/create-account">
            <Button color="primary">Create Account</Button>
          </NavLink>
        </NavItem>
  
        <!-- Log In Button -->
        <NavItem>
          <NavLink href="/log-in">
            <Button color="secondary">Log In</Button>
          </NavLink>
        </NavItem>
  
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
  