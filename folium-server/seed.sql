-- Drop tables if they already exist to start fresh
DROP TABLE IF EXISTS notes;
DROP TABLE IF EXISTS user_classes;
DROP TABLE IF EXISTS classes;
DROP TABLE IF EXISTS users;

-- Create the tables
CREATE TABLE users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    email VARCHAR(100) NOT NULL UNIQUE,
    password VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);

CREATE TABLE classes (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    description TEXT,
    instructor VARCHAR(100),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE user_classes (
    user_id INT NOT NULL,
    class_id INT NOT NULL,
    enrollment_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (user_id, class_id),
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (class_id) REFERENCES classes(id) ON DELETE CASCADE
);

CREATE TABLE notes (
    id INT AUTO_INCREMENT PRIMARY KEY,
    class_id INT NOT NULL UNIQUE,
    title VARCHAR(255) NOT NULL,
    file_path VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (class_id) REFERENCES classes(id) ON DELETE CASCADE
);

-- Insert sample users
INSERT INTO users (username, email, password) VALUES 
('john_doe', 'john@example.com', 'hashed_password_here'),
('jane_doe', 'jane@example.com', 'hashed_password_here'),
('michael_scott', 'michael@dundermifflin.com', 'hashed_password_here'),
('pam_beesly', 'pam@dundermifflin.com', 'hashed_password_here'),
('jim_halpert', 'jim@dundermifflin.com', 'hashed_password_here'),
('dwight_schrute', 'dwight@dundermifflin.com', 'hashed_password_here'),
('stanley_hudson', 'stanley@dundermifflin.com', 'hashed_password_here'),
('kevin_malone', 'kevin@dundermifflin.com', 'hashed_password_here');

-- Insert sample classes
INSERT INTO classes (name, description, instructor) VALUES 
('Operating Systems', 'Collaborative note for OS class covering processes, memory management, and synchronization.', 'Professor Smith'),
('Database Systems', 'Shared note for database concepts such as SQL, transactions, and normalization.', 'Professor Johnson'),
('Data Structures', 'Collective note on common data structures and algorithms.', 'Professor Williams'),
('Computer Networks', 'Shared note for networking fundamentals and protocols.', 'Professor Brown'),
('Algorithms', 'Collaborative space for algorithm design and analysis.', 'Professor Davis');

-- Enroll users into classes (sample enrollments)
-- Enroll all users into Operating Systems (class_id = 1)
INSERT INTO user_classes (user_id, class_id) VALUES 
(1, 1), (2, 1), (3, 1), (4, 1), (5, 1), (6, 1), (7, 1), (8, 1);

-- Enroll a subset into Database Systems (class_id = 2)
INSERT INTO user_classes (user_id, class_id) VALUES 
(1, 2), (3, 2), (5, 2), (7, 2);

-- Enroll a subset into Data Structures (class_id = 3)
INSERT INTO user_classes (user_id, class_id) VALUES 
(2, 3), (4, 3), (6, 3), (8, 3);

-- Enroll a subset into Computer Networks (class_id = 4)
INSERT INTO user_classes (user_id, class_id) VALUES 
(1, 4), (2, 4), (4, 4), (5, 4), (7, 4);

-- Enroll a subset into Algorithms (class_id = 5)
INSERT INTO user_classes (user_id, class_id) VALUES 
(3, 5), (4, 5), (6, 5), (8, 5);

-- Insert shared notes for each class. Note file paths could follow a consistent folder structure.
INSERT INTO notes (class_id, title, file_path) VALUES 
(1, 'Operating Systems Shared Note', '/notes/operating_systems/os_shared_note.txt'),
(2, 'Database Systems Shared Note', '/notes/database_systems/db_shared_note.txt'),
(3, 'Data Structures Shared Note', '/notes/data_structures/ds_shared_note.txt'),
(4, 'Computer Networks Shared Note', '/notes/computer_networks/cn_shared_note.txt'),
(5, 'Algorithms Shared Note', '/notes/algorithms/alg_shared_note.txt');
