// ledger/src/lib/stores/notification.ts
import { writable } from 'svelte/store';

export const notification = writable<string>('');
