<script lang="ts">
  import { goto } from '$app/navigation';
  import {
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
  import { getContext } from 'svelte';
  import foliumService from '../../lib/FoliumService'; // Adjusted the path

  // Get the function from context to show the connection error modal
  const showConnectionError = getContext('showConnectionError') as () => void;
  
  let username = '';
  let password = '';

  async function handleLogin() {
    console.log('Attempting to log in with:', username, password);
    try {
      // Use foliumService to handle login
      const data = await foliumService.login(username, password);
      console.log("Login successful:", data);

      // Store the token and username so your Navbar can pick them up
      sessionStorage.setItem("userToken", data.token);
      sessionStorage.setItem("username", username);

      // Redirect to the main page after login
      goto("/");
    } catch (err: any) {
      console.error("Login failed:", err);

      if (err.message.includes("401")) {
        alert("Invalid username or password");
      } else if (err.message.includes("500")) {
        showConnectionError();
      } else {
        alert("Login failed: " + err.message);
      }
    }
  }
</script>

<Container class="vh-100 d-flex justify-content-center align-items-center">
  <Row class="w-100">
    <Col sm="12" md="10" lg="6" xl="5" class="mx-auto">
      <Card class="p-3 shadow-sm">
        <CardBody>
          <CardTitle class="mb-4 text-center">
            <h2 class="m-0">Log In</h2>
          </CardTitle>
          <Form on:submit={(e) => { e.preventDefault(); handleLogin(); }}>
            <FormGroup>
              <Label for="username">Username</Label>
              <Input id="username" type="text" bind:value={username} required />
            </FormGroup>
            <FormGroup>
              <Label for="password">Password</Label>
              <Input id="password" type="password" bind:value={password} required />
            </FormGroup>
            <Button color="primary" type="submit" class="mt-3 w-100">
              Log In
            </Button>
          </Form>
        </CardBody>
      </Card>
    </Col>
  </Row>
</Container>