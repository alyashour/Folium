<script lang="ts">
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
    Spinner
  } from '@sveltestrap/sveltestrap';

  let username = "";
  let password = "";
  let confirmPassword = "";
  let error: string | null = null;
  let isLoading = false;

  async function handleRegister() {
    error = null;
    
    // Very basic validation: check that both passwords match.
    if (password !== confirmPassword) {
      error = "Passwords do not match";
      return;
    }

    isLoading = true;
    
    try {
      // Use foliumService to register the user with username and password
      await foliumService.register(username, password);

      // If registration is successful, redirect to the login page.
      goto("/log-in");
    } catch (err: any) {
      error = err.message || "Registration failed. Please try again later.";
    } finally {
      isLoading = false;
    }
  }
</script>

<Container class="vh-100 d-flex justify-content-center align-items-center">
<Row class="w-100">
  <Col sm="12" md="10" lg="6" xl="5" class="mx-auto">
    <Card class="p-3 shadow-sm">
      <CardBody>
        <CardTitle class="mb-4 text-center">
          <h2 class="m-0">Register</h2>
        </CardTitle>
        
        {#if error}
          <Alert color="danger" class="mb-4" dismissible>{error}</Alert>
        {/if}
        
        <Form on:submit={(e) => {
          e.preventDefault();
          handleRegister();
        }}>
          <FormGroup>
            <Label for="username">Username</Label>
            <Input 
              id="username" 
              type="text" 
              bind:value={username} 
              required 
            />
          </FormGroup>
          
          <FormGroup>
            <Label for="password">Password</Label>
            <Input 
              id="password" 
              type="password" 
              bind:value={password} 
              required 
            />
          </FormGroup>
          
          <FormGroup>
            <Label for="confirmPassword">Confirm Password</Label>
            <Input 
              id="confirmPassword" 
              type="password" 
              bind:value={confirmPassword} 
              required 
            />
          </FormGroup>
          
          <Button 
            color="primary" 
            type="submit" 
            class="mt-3 w-100"
            disabled={isLoading}
          >
            {#if isLoading}
              <Spinner size="sm" /> Creating Account...
            {:else}
              Create Account
            {/if}
          </Button>
        </Form>
        
        <p class="mt-2 text-center">Already have an account? <a href="/log-in">Log In</a></p>
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