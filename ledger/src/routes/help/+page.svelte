<script lang="ts">
  import { Button, Collapse, Card, CardBody } from '@sveltestrap/sveltestrap';
  
  // FAQ accordion state
  let openSection = "";
  
  // Toggle FAQ sections
  function toggleSection(id: string) {
    openSection = openSection === id ? "" : id;
  }
</script>

<div class="help-container">
  <h1>Help Center</h1>
  <p class="intro-text">
    Welcome to the Folium Help Center! Here you'll find answers to frequently asked questions 
    and resources to help you get the most out of your collaborative note-taking experience.
  </p>
  
  <div class="search-container">
    <input type="text" placeholder="Search for help topics..." class="search-input" />
    <Button color="primary">Search</Button>
  </div>
  
  <section class="quick-links">
    <h2>Quick Links</h2>
    <div class="link-cards">
      <a href="#getting-started" class="link-card">
        <span class="link-icon">🚀</span>
        <h3>Getting Started</h3>
        <p>New to Folium? Start here!</p>
      </a>
      
      <a href="#classes" class="link-card">
        <span class="link-icon">📚</span>
        <h3>Classes</h3>
        <p>Creating and joining classes</p>
      </a>
      
      <a href="#notes" class="link-card">
        <span class="link-icon">📝</span>
        <h3>Notes</h3>
        <p>Working with collaborative notes</p>
      </a>
      
      <a href="#account" class="link-card">
        <span class="link-icon">👤</span>
        <h3>Account</h3>
        <p>Managing your account</p>
      </a>
    </div>
  </section>
  
  <section class="faq-section" id="getting-started">
    <h2>Getting Started</h2>
    
    <div class="faq-item">
      <div 
        class="faq-question {openSection === 'q1' ? 'active' : ''}"
        on:click={() => toggleSection('q1')}
      >
        <span>How do I create an account?</span>
        <span class="toggle-icon">{openSection === 'q1' ? '−' : '+'}</span>
      </div>
      
      <Collapse isOpen={openSection === 'q1'}>
        <div class="faq-answer">
          <p>Creating an account on Folium is simple:</p>
          <ol>
            <li>Click the "Register" button in the top navigation bar</li>
            <li>Enter your email address, name, and create a password</li>
            <li>Verify your email address through the confirmation link we send you</li>
            <li>Complete your profile to personalize your experience</li>
          </ol>
          <p>Once registered, you can join existing classes or create your own.</p>
        </div>
      </Collapse>
    </div>
    
    <div class="faq-item">
      <div 
        class="faq-question {openSection === 'q2' ? 'active' : ''}"
        on:click={() => toggleSection('q2')}
      >
        <span>What is Folium?</span>
        <span class="toggle-icon">{openSection === 'q2' ? '−' : '+'}</span>
      </div>
      
      <Collapse isOpen={openSection === 'q2'}>
        <div class="faq-answer">
          <p>
            Folium is a collaborative note-taking platform designed specifically for students. 
            It allows classmates to create, share, and verify notes together, ensuring everyone 
            has access to comprehensive and accurate information.
          </p>
          <p>
            Our mission is to democratize knowledge within classrooms by leveraging 
            collective intelligence, making quality educational content accessible to all students.
          </p>
        </div>
      </Collapse>
    </div>
  </section>
  
  <section class="faq-section" id="classes">
    <h2>Classes</h2>
    
    <div class="faq-item">
      <div 
        class="faq-question {openSection === 'q3' ? 'active' : ''}"
        on:click={() => toggleSection('q3')}
      >
        <span>How do I create a class?</span>
        <span class="toggle-icon">{openSection === 'q3' ? '−' : '+'}</span>
      </div>
      
      <Collapse isOpen={openSection === 'q3'}>
        <div class="faq-answer">
          <p>To create a new class:</p>
          <ol>
            <li>Navigate to the "Classes" page from the navigation menu</li>
            <li>Click the "Create New Class" or "Add Class" button</li>
            <li>Enter the class name and other details when prompted</li>
            <li>Click "Create" to finish</li>
          </ol>
          <p>After creating a class, you can invite other students to join and start collaborating on notes.</p>
        </div>
      </Collapse>
    </div>
    
    <div class="faq-item">
      <div 
        class="faq-question {openSection === 'q4' ? 'active' : ''}"
        on:click={() => toggleSection('q4')}
      >
        <span>How do I join an existing class?</span>
        <span class="toggle-icon">{openSection === 'q4' ? '−' : '+'}</span>
      </div>
      
      <Collapse isOpen={openSection === 'q4'}>
        <div class="faq-answer">
          <p>You can join an existing class in two ways:</p>
          <h4>With a Class Code</h4>
          <ol>
            <li>Navigate to the "Classes" page</li>
            <li>Click "Join Class"</li>
            <li>Enter the class code provided by the class owner or another member</li>
            <li>Click "Join" to be added to the class</li>
          </ol>
          <h4>From Public Classes</h4>
          <ol>
            <li>Navigate to the "Classes" page</li>
            <li>Click "Show Public Classes"</li>
            <li>Browse the available classes</li>
            <li>Click "Join" next to the class you want to join</li>
          </ol>
        </div>
      </Collapse>
    </div>
  </section>
  
  <section class="faq-section" id="notes">
    <h2>Notes</h2>
    
    <div class="faq-item">
      <div 
        class="faq-question {openSection === 'q5' ? 'active' : ''}"
        on:click={() => toggleSection('q5')}
      >
        <span>How do collaborative notes work?</span>
        <span class="toggle-icon">{openSection === 'q5' ? '−' : '+'}</span>
      </div>
      
      <Collapse isOpen={openSection === 'q5'}>
        <div class="faq-answer">
          <p>
            Folium's collaborative notes allow multiple students to contribute to a single, 
            comprehensive document for each class. Here's how it works:
          </p>
          <ul>
            <li><strong>Adding Notes:</strong> Students can upload their own notes or directly edit the class "Big Note"</li>
            <li><strong>Integration:</strong> Individual notes are intelligently combined into the collective knowledge base</li>
            <li><strong>Verification:</strong> Students can mark information as verified or flag potential errors</li>
            <li><strong>History:</strong> All changes are tracked, so you can see who contributed what and when</li>
          </ul>
          <p>
            This collaborative approach ensures that no single student has to capture everything, 
            and that knowledge gaps from missed classes can be filled by peers.
          </p>
        </div>
      </Collapse>
    </div>
    
    <div class="faq-item">
      <div 
        class="faq-question {openSection === 'q6' ? 'active' : ''}"
        on:click={() => toggleSection('q6')}
      >
        <span>How do I upload my notes?</span>
        <span class="toggle-icon">{openSection === 'q6' ? '−' : '+'}</span>
      </div>
      
      <Collapse isOpen={openSection === 'q6'}>
        <div class="faq-answer">
          <p>To upload your notes to a class:</p>
          <ol>
            <li>Navigate to the specific class page</li>
            <li>Click the "Upload" button near the note editor</li>
            <li>Select your file (supported formats include PDF, Markdown, and text files)</li>
            <li>Optionally add a title or description for your notes</li>
            <li>Click "Upload" to submit</li>
          </ol>
          <p>
            Your notes will be processed and integrated into the class's Big Note. 
            The system will attempt to merge your content intelligently with existing notes.
          </p>
        </div>
      </Collapse>
    </div>
  </section>
  
  <section class="faq-section" id="account">
    <h2>Account Management</h2>
    
    <div class="faq-item">
      <div 
        class="faq-question {openSection === 'q7' ? 'active' : ''}"
        on:click={() => toggleSection('q7')}
      >
        <span>How do I reset my password?</span>
        <span class="toggle-icon">{openSection === 'q7' ? '−' : '+'}</span>
      </div>
      
      <Collapse isOpen={openSection === 'q7'}>
        <div class="faq-answer">
          <p>If you've forgotten your password:</p>
          <ol>
            <li>Click on "Log In" in the navigation menu</li>
            <li>Select "Forgot Password" below the login form</li>
            <li>Enter the email address associated with your account</li>
            <li>Check your email for a reset link</li>
            <li>Follow the link and create a new password</li>
          </ol>
          <p>If you're already logged in and want to change your password:</p>
          <ol>
            <li>Click on your profile icon in the top right</li>
            <li>Select "Account Settings"</li>
            <li>Navigate to the "Password" section</li>
            <li>Enter your current password and your new password</li>
            <li>Click "Save Changes"</li>
          </ol>
        </div>
      </Collapse>
    </div>
    
    <div class="faq-item">
      <div 
        class="faq-question {openSection === 'q8' ? 'active' : ''}"
        on:click={() => toggleSection('q8')}
      >
        <span>How do I delete my account?</span>
        <span class="toggle-icon">{openSection === 'q8' ? '−' : '+'}</span>
      </div>
      
      <Collapse isOpen={openSection === 'q8'}>
        <div class="faq-answer">
          <p>
            We're sorry to see you go! To delete your account:
          </p>
          <ol>
            <li>Click on your profile icon in the top right</li>
            <li>Select "Account Settings"</li>
            <li>Scroll to the bottom of the page</li>
            <li>Click "Delete Account"</li>
            <li>Confirm your decision by entering your password</li>
          </ol>
          <p>
            <strong>Important:</strong> Account deletion is permanent and cannot be undone. 
            All your personal data will be removed from our systems, though your contributions 
            to class notes will remain anonymized.
          </p>
        </div>
      </Collapse>
    </div>
  </section>
  
  <section class="contact-section">
    <h2>Still Need Help?</h2>
    <p>
      If you couldn't find what you were looking for, you can reach out to our support team.
    </p>
    <div class="contact-options">
      <Button color="primary" href="/contact">Contact Support</Button>
      <span class="or">or</span>
      <a href="mailto:support@folium.com">support@folium.com</a>
    </div>
  </section>
</div>

<style>
  .help-container {
    max-width: 900px;
    margin: 0 auto;
    padding: 2rem 1rem;
  }
  
  h1 {
    margin-bottom: 1rem;
  }
  
  .intro-text {
    font-size: 1.1rem;
    margin-bottom: 2rem;
  }
  
  .search-container {
    display: flex;
    margin-bottom: 3rem;
    max-width: 600px;
  }
  
  .search-input {
    flex: 1;
    padding: 0.5rem 1rem;
    font-size: 1rem;
    border: 1px solid #ddd;
    border-radius: 4px 0 0 4px;
    border-right: none;
  }
  
  .search-container :global(button) {
    border-radius: 0 4px 4px 0;
  }
  
  section {
    margin-bottom: 3rem;
  }
  
  h2 {
    margin-bottom: 1.5rem;
    padding-bottom: 0.5rem;
    border-bottom: 1px solid #eee;
  }
  
  /* Quick links section */
  .link-cards {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));
    gap: 1.5rem;
  }
  
  .link-card {
    display: block;
    padding: 1.5rem;
    background-color: #f8f9fa;
    border-radius: 8px;
    text-decoration: none;
    color: inherit;
    transition: transform 0.2s, box-shadow 0.2s;
    text-align: center;
  }
  
  .link-card:hover {
    transform: translateY(-3px);
    box-shadow: 0 4px 8px rgba(0,0,0,0.1);
  }
  
  .link-icon {
    font-size: 2rem;
    display: block;
    margin-bottom: 1rem;
  }
  
  .link-card h3 {
    margin: 0 0 0.5rem 0;
    color: #007bff;
  }
  
  .link-card p {
    margin: 0;
    font-size: 0.9rem;
    color: #666;
  }
  
  /* FAQ section */
  .faq-item {
    margin-bottom: 1rem;
    border: 1px solid #eee;
    border-radius: 8px;
    overflow: hidden;
  }
  
  .faq-question {
    padding: 1rem;
    background-color: #f8f9fa;
    cursor: pointer;
    display: flex;
    justify-content: space-between;
    align-items: center;
    font-weight: 500;
    transition: background-color 0.2s;
  }
  
  .faq-question:hover {
    background-color: #e9ecef;
  }
  
  .faq-question.active {
    background-color: #e7f1ff;
    color: #0056b3;
  }
  
  .toggle-icon {
    font-size: 1.5rem;
    line-height: 1;
  }
  
  .faq-answer {
    padding: 1rem;
    background-color: #fff;
    border-top: 1px solid #eee;
  }
  
  .faq-answer h4 {
    margin: 1rem 0 0.5rem;
  }
  
  .faq-answer p:last-child {
    margin-bottom: 0;
  }
  
  /* Contact section */
  .contact-section {
    background-color: #f8f9fa;
    padding: 2rem;
    border-radius: 8px;
    text-align: center;
  }
  
  .contact-options {
    display: flex;
    justify-content: center;
    align-items: center;
    flex-wrap: wrap;
    gap: 1rem;
    margin-top: 1.5rem;
  }
  
  .or {
    color: #666;
  }
  
  @media (max-width: 576px) {
    .search-container {
      flex-direction: column;
    }
    
    .search-input {
      border-right: 1px solid #ddd;
      border-radius: 4px;
      margin-bottom: 0.5rem;
    }
    
    .search-container :global(button) {
      border-radius: 4px;
      width: 100%;
    }
  }
</style>