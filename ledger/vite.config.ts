import { defineConfig } from 'vite';
import { sveltekit } from '@sveltejs/kit/vite';

export default defineConfig({
  plugins: [sveltekit()],
  server: {
    proxy: {
      // Any request to /api will be proxied to your API base URL.
      '/api': {
        target: process.env.PUBLIC_API_BASE_URL,
        changeOrigin: true,
        secure: false
      }
    }
  }
});
