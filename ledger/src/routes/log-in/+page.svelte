<script lang="ts">
  import { goto } from '$app/navigation';
  import { PUBLIC_API_BASE_URL } from '$env/static/public';
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

  // Get the function from context to show the connection error modal
  const showConnectionError = getContext('showConnectionError') as () => void;

  let username = '';
  let password = '';

  async function handleLogin() {
    console.log('Attempting to log in with:', username, password);
    try {
      const res = await fetch(`${PUBLIC_API_BASE_URL}/api/auth/login`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ username, password })
      });

      if (!res.ok) {
        if (res.status === 401) {
          console.log("Invalid credentials");
          alert("Invalid username or password");
        } else if (res.status >= 500) {
          console.log("Server error occurred");
          showConnectionError();
        } else {
          console.log(`Server responded with status ${res.status}`);
          alert("Login failed with status " + res.status);
        }
        return;
      }

    const data = await res.json();
    console.log("Login successful:", data);

    // Store the token and username so your Navbar can pick them up
    sessionStorage.setItem("userToken", data.token);
    sessionStorage.setItem("username", username);

    // Redirect to the main page after login
    goto("/");
  } catch (err) {
    console.error("Network error => unreachable server", err);
    showConnectionError();
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

<style>
  .vh-100 {
    height: 100vh;
  }
  .card {
    max-width: 500px;
    margin: auto;
  }
</style>
