<script lang="ts">
  import { Button, Card, CardBody } from '@sveltestrap/sveltestrap';
  import { goto } from '$app/navigation';
  
  // Team members
  const team = [
    { name: "Omar Ramadan", role: "", image: '' },
    { name: "Aly", role: "", image: '' },
    { name: "Saad", role: "", image: '' },
    { name: "Hamza", role: "", image: '' }
  ];
  
  // Features data
  const features = [
    {
      title: "Collaborative Notes",
      description: "Create, edit, and verify notes with classmates in real-time, ensuring accuracy and completeness.",
      icon: "📝"
    },
    {
      title: "Class Communities",
      description: "Join class-specific communities where knowledge and resources are centralized for every student.",
      icon: "👥"
    },
    {
      title: "Knowledge Verification",
      description: "Upvote accurate information and flag incorrect content, creating reliable study resources.",
      icon: "✅"
    },
    {
      title: "Resource Sharing",
      description: "Share supplemental materials, links, and resources to enhance everyone's understanding.",
      icon: "🔄"
    }
  ];
</script>

<div class="about-container">
  <header class="hero">
    <h1>Our Mission</h1>
    <p class="tagline">Democratizing Knowledge for All Students</p>
    
    <div class="mission-statement">
      <p>
        At Folium, we believe that education should be accessible, collaborative, and equitable. 
        We're building a platform where students can collectively create, refine, and access 
        comprehensive knowledge for their classes - ensuring that quality educational content 
        is available to everyone, regardless of their note-taking abilities or class attendance.
      </p>
    </div>
  </header>
  
  <section class="vision-section">
    <h2>Our Vision</h2>
    <div class="vision-content">
      <div class="vision-image">
        <div class="image-placeholder">
          <span>📚</span>
        </div>
      </div>
      <div class="vision-text">
        <p>
          Imagine a world where every student has access to complete, accurate, and well-organized notes
          for every class they take. A world where knowledge gaps from missed lectures or confusing concepts
          are filled through collective intelligence.
        </p>
        <p>
          Folium makes this vision a reality by providing a centralized platform where class communities
          collaborate to build comprehensive knowledge bases that evolve and improve over time.
        </p>
        <p>
          Our platform transforms individual study materials into collective wisdom, creating an 
          ecosystem where every student contributes to and benefits from shared knowledge.
        </p>
      </div>
    </div>
  </section>
  
  <section class="how-it-works">
    <h2>How Folium Works</h2>
    
    <div class="steps">
      <div class="step">
        <div class="step-number">1</div>
        <h3>Join Your Class</h3>
        <p>Connect with classmates in a dedicated space for your specific course.</p>
      </div>
      
      <div class="step">
        <div class="step-number">2</div>
        <h3>Contribute Notes</h3>
        <p>Upload your notes or create new content directly in our collaborative editor.</p>
      </div>
      
      <div class="step">
        <div class="step-number">3</div>
        <h3>Collaborate & Verify</h3>
        <p>Work together to refine, expand, and verify the accuracy of class materials.</p>
      </div>
      
      <div class="step">
        <div class="step-number">4</div>
        <h3>Access Anywhere</h3>
        <p>Study from comprehensive, verified notes anytime, on any device.</p>
      </div>
    </div>
  </section>
  
  <section class="features-section">
    <h2>Key Features</h2>
    <div class="features-grid">
      {#each features as feature}
        <div class="feature-card">
          <div class="feature-icon">{feature.icon}</div>
          <h3>{feature.title}</h3>
          <p>{feature.description}</p>
        </div>
      {/each}
    </div>
  </section>
  
  <section class="testimonials">
    <h2>What Students Say</h2>
    <div class="testimonial-slider">
      <div class="testimonial">
        <blockquote>
          "Folium completely changed how I study. Having access to comprehensive class notes that I can trust has boosted my confidence and improved my grades."
        </blockquote>
        <cite>— Jamie K., Computer Science Major</cite>
      </div>
      
      <div class="testimonial">
        <blockquote>
          "As someone who struggles with note-taking, Folium has been a lifesaver. I can focus on understanding concepts in class rather than frantically writing everything down."
        </blockquote>
        <cite>— Taylor M., Biology Student</cite>
      </div>
      
      <div class="testimonial">
        <blockquote>
          "The collaborative aspect is amazing! When I miss something important, my classmates have already filled in the gaps by the time I check Folium."
        </blockquote>
        <cite>— Riley P., Business Administration</cite>
      </div>
    </div>
  </section>
  
  <section class="team-section">
    <h2>Our Team</h2>
    <p class="team-intro">
      We're a passionate group of students, educators, and developers committed to transforming 
      the way students learn and collaborate.
    </p>
    
    <div class="team-grid">
      {#each team as member}
        <div class="team-member">
          <div class="member-photo" style="background-image: url({member.image})">
            <div class="member-photo-placeholder">{member.name[0]}</div>
          </div>
          <h3>{member.name}</h3>
          <p>{member.role}</p>
        </div>
      {/each}
    </div>
  </section>
  
  <section class="cta-section">
    <h2>Ready to Transform How You Learn?</h2>
    <p>Join thousands of students already collaborating on Folium.</p>
    <div class="cta-buttons">
      <Button color="primary" size="lg" on:click={() => goto('/register')}>Sign Up Now</Button>
      <Button outline color="secondary" size="lg" on:click={() => goto('/log-in')}>Log In</Button>
    </div>
  </section>
</div>

<style>
  .about-container {
    max-width: 1200px;
    margin: 0 auto;
    padding: 2rem 1rem;
  }
  
  .hero {
    text-align: center;
    margin-bottom: 4rem;
  }
  
  .hero h1 {
    font-size: 3rem;
    margin-bottom: 0.5rem;
    color: #1a73e8;
  }
  
  .tagline {
    font-size: 1.5rem;
    color: #555;
    margin-bottom: 2rem;
  }
  
  .mission-statement {
    max-width: 800px;
    margin: 0 auto;
    font-size: 1.2rem;
    line-height: 1.8;
  }
  
  section {
    margin-bottom: 5rem;
  }
  
  h2 {
    font-size: 2rem;
    margin-bottom: 2rem;
    text-align: center;
    color: #333;
    position: relative;
  }
  
  h2::after {
    content: '';
    display: block;
    width: 60px;
    height: 4px;
    background-color: #1a73e8;
    margin: 0.5rem auto;
  }
  
  /* Vision section */
  .vision-content {
    display: flex;
    align-items: center;
    gap: 3rem;
  }
  
  .vision-image {
    flex: 0 0 40%;
  }
  
  .image-placeholder {
    background-color: #e8f0fe;
    border-radius: 10px;
    padding: 2rem;
    display: flex;
    align-items: center;
    justify-content: center;
    height: 300px;
  }
  
  .image-placeholder span {
    font-size: 6rem;
  }
  
  .vision-text {
    flex: 1;
  }
  
  .vision-text p {
    margin-bottom: 1.5rem;
    font-size: 1.1rem;
    line-height: 1.6;
  }
  
  /* How it works section */
  .steps {
    display: flex;
    justify-content: space-between;
    flex-wrap: wrap;
    gap: 1.5rem;
  }
  
  .step {
    flex: 1 1 200px;
    background-color: #f8f9fa;
    padding: 2rem;
    border-radius: 10px;
    position: relative;
    box-shadow: 0 4px 6px rgba(0,0,0,0.05);
  }
  
  .step-number {
    position: absolute;
    top: -20px;
    left: 20px;
    width: 40px;
    height: 40px;
    background-color: #1a73e8;
    color: white;
    border-radius: 50%;
    display: flex;
    align-items: center;
    justify-content: center;
    font-weight: bold;
    font-size: 1.2rem;
  }
  
  .step h3 {
    margin-bottom: 1rem;
    color: #1a73e8;
  }
  
  /* Features section */
  .features-grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(240px, 1fr));
    gap: 2rem;
  }
  
  .feature-card {
    background-color: white;
    padding: 1.5rem;
    border-radius: 10px;
    box-shadow: 0 4px 12px rgba(0,0,0,0.08);
    transition: transform 0.3s ease;
  }
  
  .feature-card:hover {
    transform: translateY(-5px);
  }
  
  .feature-icon {
    font-size: 2.5rem;
    margin-bottom: 1rem;
  }
  
  .feature-card h3 {
    color: #1a73e8;
    margin-bottom: 0.75rem;
  }
  
  /* Testimonials section */
  .testimonials {
    background-color: #f8f9fa;
    padding: 3rem;
    border-radius: 10px;
  }
  
  .testimonial-slider {
    display: flex;
    flex-wrap: wrap;
    gap: 2rem;
  }
  
  .testimonial {
    flex: 1 1 300px;
    background-color: white;
    padding: 1.5rem;
    border-radius: 10px;
    box-shadow: 0 2px 8px rgba(0,0,0,0.05);
  }
  
  blockquote {
    font-style: italic;
    color: #555;
    margin-bottom: 1rem;
    line-height: 1.6;
  }
  
  cite {
    font-weight: bold;
    display: block;
    text-align: right;
  }
  
  /* Team section */
  .team-intro {
    text-align: center;
    max-width: 700px;
    margin: 0 auto 3rem auto;
  }
  
  .team-grid {
    display: flex;
    flex-wrap: wrap;
    justify-content: center;
    gap: 2rem;
  }
  
  .team-member {
    flex: 0 0 200px;
    text-align: center;
  }
  
  .member-photo {
    width: 150px;
    height: 150px;
    border-radius: 50%;
    margin: 0 auto 1rem auto;
    background-size: cover;
    background-position: center;
    display: flex;
    align-items: center;
    justify-content: center;
    background-color: #e8f0fe;
  }
  
  .member-photo-placeholder {
    font-size: 3rem;
    color: #1a73e8;
    font-weight: bold;
  }
  
  .team-member h3 {
    margin-bottom: 0.25rem;
  }
  
  .team-member p {
    color: #666;
  }
  
  /* CTA section */
  .cta-section {
    text-align: center;
    background-color: #e8f0fe;
    padding: 3rem;
    border-radius: 10px;
  }
  
  .cta-buttons {
    display: flex;
    justify-content: center;
    gap: 1rem;
    margin-top: 2rem;
    flex-wrap: wrap;
  }
  
  /* Responsive styles */
  @media (max-width: 768px) {
    .vision-content {
      flex-direction: column;
      gap: 1.5rem;
    }
    
    .hero h1 {
      font-size: 2.5rem;
    }
    
    .tagline {
      font-size: 1.2rem;
    }
    
    .testimonial-slider {
      flex-direction: column;
    }
  }
</style>