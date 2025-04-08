<script lang="ts">
    import { browser } from '$app/environment';
    import { PUBLIC_API_BASE_URL } from '$env/static/public';
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
      Row
    } from '@sveltestrap/sveltestrap';
    import { onMount } from 'svelte';
  
    let username = "";
    let oldPassword = "";
    let newPassword = "";
    let confirmPassword = "";
    let successMessage = "";
    let errorMessage = "";
  
    // onMount: Load the username from sessionStorage (only available in the browser)
    onMount(() => {
      if (browser) {
        username = sessionStorage.getItem("username") || "";
      }
    });
  
    async function handleChangePassword() {
      // Clear previous messages
      errorMessage = "";
      successMessage = "";
      
      // Check that the new passwords match
      if (newPassword !== confirmPassword) {
        errorMessage = "New password and confirmation do not match.";
        return;
      }
  
      try {
        // Optionally grab the token from sessionStorage if needed (for Authorization)
        const token = browser ? sessionStorage.getItem("userToken") : null;
        const res = await fetch(`${PUBLIC_API_BASE_URL}/api/auth/change-password`, {
          method: "POST",
          headers: {
            "Content-Type": "application/json",
            ...(token ? { "Authorization": "Bearer " + token } : {})
          },
          body: JSON.stringify({ username, old_password: oldPassword, new_password: newPassword })
        });
  
        if (!res.ok) {
          if (res.status === 401) { // Most likely invalid old password (or missing token)
            const data = await res.json();
            errorMessage = data.message || "Invalid credentials.";
          } else {
            errorMessage = `Server error with status ${res.status}.`;
          }
          return;
        }
  
        const data = await res.json();
        successMessage = data.message || "Password changed successfully.";
        
        // Optionally clear form fields
        oldPassword = "";
        newPassword = "";
        confirmPassword = "";
      } catch (err) {
        errorMessage = "Network error: Unable to connect to server.";
        console.error(err);
      }
    }
  </script>
  
  <Container class="vh-100 d-flex justify-content-center align-items-center">
    <Row class="w-100">
      <Col sm="12" md="10" lg="6" xl="5" class="mx-auto">
        <Card class="p-3 shadow-sm">
          <CardBody>
            <CardTitle class="mb-4 text-center">
              <h2 class="m-0">Profile</h2>
            </CardTitle>
            {#if successMessage}
              <Alert color="success">{successMessage}</Alert>
            {/if}
            {#if errorMessage}
              <Alert color="danger">{errorMessage}</Alert>
            {/if}
            <Form on:submit={(e) => { e.preventDefault(); handleChangePassword(); }}>
              <FormGroup>
                <Label for="username">Username</Label>
                <!-- Display username as read-only -->
                <Input id="username" type="text" value={username} readonly />
              </FormGroup>
              <FormGroup>
                <Label for="oldPassword">Old Password</Label>
                <Input id="oldPassword" type="password" bind:value={oldPassword} required />
              </FormGroup>
              <FormGroup>
                <Label for="newPassword">New Password</Label>
                <Input id="newPassword" type="password" bind:value={newPassword} required />
              </FormGroup>
              <FormGroup>
                <Label for="confirmPassword">Confirm New Password</Label>
                <Input id="confirmPassword" type="password" bind:value={confirmPassword} required />
              </FormGroup>
              <Button color="primary" type="submit" class="mt-3 w-100">
                Change Password
              </Button>
            </Form>
          </CardBody>
        </Card>
      </Col>
    </Row>
  </Container>
  
  <style>
    .vh-100 {
      height: 100vh;
    }
    .card {
      max-width: 500px;
      margin: auto;
    }
  </style>
  