<script lang="ts">
  import { Button, Form, FormGroup, Label, Input, Alert } from '@sveltestrap/sveltestrap';
  
  // Form state
  let name = "";
  let email = "";
  let subject = "";
  let message = "";
  let submitted = false;
  let error = false;
  let loading = false;
  
  // Submit form handler
  async function handleSubmit(event: Event) {
    if (event) event.preventDefault();
    loading = true;
    error = false;
    
    if (!name || !email || !message) {
      error = true;
      loading = false;
      return;
    }
    
    // Simulate form submission with a delay
    setTimeout(() => {
      // In a real application, you would send this data to your backend
      console.log({ name, email, subject, message });
      
      // Reset form and show success message
      submitted = true;
      loading = false;
      name = "";
      email = "";
      subject = "";
      message = "";
      
      // Reset success message after 5 seconds
      setTimeout(() => {
        submitted = false;
      }, 5000);
    }, 800);
  }
</script>

<div class="contact-container">
  <h1>Contact Us</h1>
  
  <div class="contact-content">
    <div class="contact-info">
      <p>
        Have questions about Folium or need help with your account? 
        We're here to help! Fill out the form or contact us directly.
      </p>
      
      <div class="contact-methods">
        <div class="contact-method">
          <span class="icon">📧</span>
          <div>
            <h3>Email</h3>
            <p><a href="mailto:support@folium.com">support@folium.com</a></p>
          </div>
        </div>
        
        <div class="contact-method">
          <span class="icon">⏱️</span>
          <div>
            <h3>Response Time</h3>
            <p>We typically respond within 24-48 hours</p>
          </div>
        </div>
      </div>
    </div>
    
    <div class="contact-form">
      {#if submitted}
        <Alert color="success">
          Thank you for your message! We'll get back to you soon.
        </Alert>
        <Form on:submit={handleSubmit}>
          {#if error}
            <Alert color="danger">
              Please fill out all required fields.
            </Alert>
          {/if}
          
          <FormGroup>
            <Label for="name">Name <span class="required">*</span></Label>
            <Input
              type="text"
              id="name"
              bind:value={name}
              placeholder="Your name"
              required
            />
          </FormGroup>
          
          <FormGroup>
            <Label for="email">Email <span class="required">*</span></Label>
            <Input
              type="email"
              id="email"
              bind:value={email}
              placeholder="your.email@example.com"
              required
            />
          </FormGroup>
          
          <FormGroup>
            <Label for="subject">Subject</Label>
            <Input
              type="text"
              id="subject"
              bind:value={subject}
              placeholder="What is this regarding?"
            />
          </FormGroup>
          
          <FormGroup>
            <Label for="message">Message <span class="required">*</span></Label>
            <Input
              type="textarea"
              id="message"
              bind:value={message}
              placeholder="How can we help you?"
              rows={5}
              required
            />
          </FormGroup>
          
          <Button type="submit" color="primary" disabled={loading}>
            {#if loading}
              Sending...
            {:else}
              Send Message
            {/if}
          </Button>
        </Form>
      {/if}
    </div>
  </div>
</div>

<style>
  .contact-container {
    max-width: 1000px;
    margin: 0 auto;
    padding: 2rem 1rem;
  }
  
  h1 {
    margin-bottom: 1.5rem;
    color: #333;
  }
  
  .contact-content {
    display: flex;
    gap: 2rem;
    flex-wrap: wrap;
  }
  
  .contact-info {
    flex: 1;
    min-width: 300px;
  }
  
  .contact-form {
    flex: 2;
    min-width: 300px;
  }
  
  .contact-methods {
    margin-top: 2rem;
  }
  
  .contact-method {
    display: flex;
    align-items: flex-start;
    margin-bottom: 1.5rem;
  }
  
  .icon {
    font-size: 1.5rem;
    margin-right: 1rem;
    width: 40px;
    height: 40px;
    display: flex;
    align-items: center;
    justify-content: center;
    background-color: #f8f9fa;
    border-radius: 50%;
  }
  
  .contact-method h3 {
    margin: 0;
    font-size: 1.1rem;
  }
  
  .contact-method p {
    margin: 0.25rem 0 0 0;
  }
  
  .required {
    color: #dc3545;
  }
  
  @media (max-width: 768px) {
    .contact-content {
      flex-direction: column;
    }
  }
</style>