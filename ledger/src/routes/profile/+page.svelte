<script lang="ts">
  import { browser } from '$app/environment';
  import { goto } from '$app/navigation';
  import foliumService from '../../lib/FoliumService';
  import {
    Alert,
    Button,
    Card,
    CardBody,
    CardTitle,
    Col,
    Container,
    Form,
    FormGroup,
    Input,
    Label,
    Row,
    Nav,
    NavItem,
    NavLink,
    TabContent,
    TabPane
  } from '@sveltestrap/sveltestrap';
  import { onMount } from 'svelte';

  // User information
  let username = "";
  let email = "";
  let firstName = "";
  let lastName = "";
  let bio = "";
  
  // Password change fields
  let oldPassword = "";
  let newPassword = "";
  let confirmPassword = "";
  
  // UI state
  let successMessage = "";
  let errorMessage = "";
  let activeTab = "profile";
  let token = "";
  let isLoading = false;

  // onMount: Load the user data from sessionStorage
  onMount(() => {
    if (browser) {
      username = sessionStorage.getItem("username") || "";
      email = sessionStorage.getItem("userEmail") || "";
      token = sessionStorage.getItem("userToken") || "";
      firstName = sessionStorage.getItem("firstName") || "";
      lastName = sessionStorage.getItem("lastName") || "";
      
      // If we don't have a token, redirect to login
      if (!token) {
        goto('/log-in');
      }
    }
  });

  // Function to toggle between tabs
  function toggleTab(tabId: string) {
    activeTab = tabId;
    // Clear messages when switching tabs
    successMessage = "";
    errorMessage = "";
  }

  // Handle profile update
  async function handleProfileUpdate() {
    errorMessage = "";
    successMessage = "";
    isLoading = true;
    
    try {
      // In a real implementation, this would call an API endpoint to update profile info
      // For now, just simulate success
      await new Promise(resolve => setTimeout(resolve, 800));
      
      // Update sessionStorage with new values
      if (browser) {
        sessionStorage.setItem("firstName", firstName);
        sessionStorage.setItem("lastName", lastName);
        if (email !== sessionStorage.getItem("userEmail")) {
          sessionStorage.setItem("userEmail", email);
        }
      }
      
      successMessage = "Profile updated successfully.";
    } catch (err: any) {
      errorMessage = err.message || "Failed to update profile.";
      console.error(err);
    } finally {
      isLoading = false;
    }
  }

  // Handle password change using the foliumService
  async function handleChangePassword() {
    // Clear previous messages
    errorMessage = "";
    successMessage = "";
    
    // Check that the new passwords match
    if (newPassword !== confirmPassword) {
      errorMessage = "New password and confirmation do not match.";
      return;
    }
    
    isLoading = true;
    
    try {
      // Use foliumService to change password
      await foliumService.changePassword(oldPassword, newPassword);
      
      successMessage = "Password changed successfully.";
      
      // Clear form fields
      oldPassword = "";
      newPassword = "";
      confirmPassword = "";
    } catch (err: any) {
      errorMessage = err.message || "Failed to change password.";
      console.error(err);
    } finally {
      isLoading = false;
    }
  }
  
  // Handle logout using the foliumService
  async function handleLogout() {
    isLoading = true;
    
    try {
      // Use foliumService to logout
      await foliumService.logout(token);
      
      // Clear session storage
      if (browser) {
        sessionStorage.removeItem("username");
        sessionStorage.removeItem("userEmail");
        sessionStorage.removeItem("userToken");
        sessionStorage.removeItem("firstName");
        sessionStorage.removeItem("lastName");
      }
      
      // Redirect to home page
      goto('/');
    } catch (err: any) {
      errorMessage = err.message || "Failed to logout.";
      console.error(err);
      isLoading = false;
    }
  }
  
  // Handle account deletion (just a placeholder function)
  function handleDeleteAccount() {
    if (confirm("Are you sure you want to delete your account? This action cannot be undone.")) {
      // In a real app, this would call an API to delete the account
      alert("Account deletion would happen here (not implemented)");
    }
  }
</script>

<Container class="my-5">
  <Row>
    <Col sm="12" md="4" lg="3" class="mb-4">
      <Card class="profile-sidebar shadow-sm">
        <CardBody>
          <div class="profile-avatar text-center mb-4">
            <div class="avatar-circle">
              {#if firstName && lastName}
                <span class="initials">
                  {firstName[0]}{lastName[0]}
                </span>
              {:else if username}
                <span class="initials">
                  {username[0].toUpperCase()}
                </span>
              {:else}
                <span class="initials">?</span>
              {/if}
            </div>
            <h4 class="mt-3 mb-1">
              {firstName && lastName ? `${firstName} ${lastName}` : username}
            </h4>
            <p class="text-muted">{username}</p>
          </div>
          
          <Nav vertical pills>
            <NavItem>
              <NavLink 
                active={activeTab === 'profile'} 
                href="#!"
                on:click={(event) => { event.preventDefault(); toggleTab('profile'); }}
              >
                <i class="fas fa-user mr-2"></i> Profile Information
              </NavLink>
            </NavItem>
            <NavItem>
              <NavLink 
                active={activeTab === 'security'} 
                href="#!"
                on:click={(event) => { event.preventDefault(); toggleTab('security'); }}
              >
                <i class="fas fa-lock mr-2"></i> Security
              </NavLink>
            </NavItem>
            <NavItem>
              <NavLink 
                href="#!"
                on:click={(event) => { event.preventDefault(); handleLogout(); }}
                class="text-danger"
              >
                <i class="fas fa-sign-out-alt mr-2"></i> Logout
              </NavLink>
            </NavItem>
          </Nav>
        </CardBody>
      </Card>
    </Col>
    
    <Col sm="12" md="8" lg="9">
      <Card class="shadow-sm">
        <CardBody>
          {#if successMessage}
            <Alert color="success" dismissible>{successMessage}</Alert>
          {/if}
          {#if errorMessage}
            <Alert color="danger" dismissible>{errorMessage}</Alert>
          {/if}
          
          {#if activeTab === "profile"}
            <!-- Profile Information Tab -->
            <div>
              <CardTitle class="mb-4">Profile Information</CardTitle>
              <Form on:submit={(event) => { event.preventDefault(); handleProfileUpdate(); }}>
                <Row>
                  <Col md="6">
                    <FormGroup>
                      <Label for="firstName">First Name</Label>
                      <Input
                        id="firstName"
                        type="text"
                        bind:value={firstName}
                        placeholder="Enter your first name"
                      />
                    </FormGroup>
                  </Col>
                  <Col md="6">
                    <FormGroup>
                      <Label for="lastName">Last Name</Label>
                      <Input
                        id="lastName"
                        type="text"
                        bind:value={lastName}
                        placeholder="Enter your last name"
                      />
                    </FormGroup>
                  </Col>
                </Row>
                
                <FormGroup>
                  <Label for="username">Username</Label>
                  <Input
                    id="username"
                    type="text"
                    value={username}
                    readonly
                  />
                </FormGroup>
                
                <FormGroup>
                  <Label for="email">Email Address</Label>
                  <Input
                    id="email"
                    type="email"
                    bind:value={email}
                    placeholder="Enter your email address"
                  />
                </FormGroup>
                
                <FormGroup>
                  <Label for="bio">Bio</Label>
                  <Input
                    id="bio"
                    type="textarea"
                    bind:value={bio}
                    placeholder="Tell us about yourself"
                    rows={4}
                  />
                </FormGroup>
                
                <Button color="primary" type="submit" disabled={isLoading}>
                  {isLoading ? 'Saving...' : 'Save Changes'}
                </Button>
              </Form>
            </div>
          {:else if activeTab === "security"}
            <!-- Security Tab -->
            <div>
              <CardTitle class="mb-4">Security Settings</CardTitle>              
              <div class="mb-5">
                <h5>Change Password</h5>
                <hr>
                <Form on:submit={(event) => { event.preventDefault(); handleChangePassword(); }}>
                  <FormGroup>
                    <Label for="oldPassword">Current Password</Label>
                    <Input
                      id="oldPassword"
                      type="password"
                      bind:value={oldPassword}
                      required
                    />
                  </FormGroup>
                  
                  <FormGroup>
                    <Label for="newPassword">New Password</Label>
                    <Input
                      id="newPassword"
                      type="password"
                      bind:value={newPassword}
                      required
                    />
                  </FormGroup>
                  
                  <FormGroup>
                    <Label for="confirmPassword">Confirm New Password</Label>
                    <Input
                      id="confirmPassword"
                      type="password"
                      bind:value={confirmPassword}
                      required
                    />
                  </FormGroup>
                  
                  <Button color="primary" type="submit" disabled={isLoading}>
                    {isLoading ? 'Changing Password...' : 'Change Password'}
                  </Button>
                </Form>
              </div>
              
              <div class="mt-5">
                <h5 class="text-danger">Danger Zone</h5>
                <hr>
                <p>
                  Deleting your account is permanent and cannot be undone. 
                  All your personal information will be removed.
                </p>
                <Button color="danger" on:click={handleDeleteAccount}>
                  Delete Account
                </Button>
              </div>
            </div>
          {/if}
        </CardBody>
      </Card>
    </Col>
  </Row>
</Container>

<style>
  .profile-sidebar {
    height: 100%;
  }
  
  .avatar-circle {
    width: 100px;
    height: 100px;
    background-color: #007bff;
    border-radius: 50%;
    display: flex;
    align-items: center;
    justify-content: center;
    margin: 0 auto;
  }
  
  .initials {
    font-size: 2rem;
    color: white;
    font-weight: bold;
    text-transform: uppercase;
  }
  
  :global(.nav-link.active) {
    background-color: #007bff !important;
  }
  
  :global(.nav-link) {
    color: #495057;
    border-radius: 4px !important;
    margin-bottom: 5px;
  }
  
  :global(.nav-link:hover:not(.active)) {
    background-color: #f8f9fa;
  }
</style>