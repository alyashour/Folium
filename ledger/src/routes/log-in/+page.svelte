<script lang="ts">
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

  let username = '';
  let password = '';

  async function handleLogin() {
    // Replace with your actual login logic
    console.log('Attempting to log in with:', username, password);
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
  /* Ensure the container takes up full viewport height */
  .vh-100 {
    height: 100vh;
  }
  /* Optional: Increase card padding and set a max-width for a larger look */
  .card {
    max-width: 500px;
    margin: auto;
  }
</style>
