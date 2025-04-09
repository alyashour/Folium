-- Drop tables if they already exist to start fresh
DROP TABLE IF EXISTS notes;
DROP TABLE IF EXISTS user_classes;
DROP TABLE IF EXISTS classes;
DROP TABLE IF EXISTS users;

-- Create the tables
CREATE TABLE users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    password VARCHAR(255) NOT NULL,  -- A hashed password
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);

CREATE TABLE classes (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    name VARCHAR(100) NOT NULL,
    description TEXT,
    instructor VARCHAR(100),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
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
    class_id INT NOT NULL UNIQUE,  -- Each class has one and only one shared note.
    title VARCHAR(255) NOT NULL,
    file_path VARCHAR(255) NOT NULL,  -- Stores the path to the shared note file.
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (class_id) REFERENCES classes(id) ON DELETE CASCADE
);

-- Insert sample users
INSERT INTO users (username, password) VALUES 
('admin', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),
('jane_doe', 'hashed_password_here'),
('michael_scott', 'hashed_password_here'),
('pam_beesly', 'hashed_password_here'),
('jim_halpert', 'hashed_password_here'),
('dwight_schrute', 'hashed_password_here'),
('stanley_hudson', 'hashed_password_here'),
('kevin_malone', 'hashed_password_here');

-- Insert sample classes with a creator's user_id
INSERT INTO classes (user_id, name, description, instructor) VALUES 
(1, 'Operating Systems', 'Collaborative note for OS class covering processes, memory management, and synchronization.', 'Professor Smith'),
(2, 'Database Systems', 'Shared note for database concepts such as SQL, transactions, and normalization.', 'Professor Johnson'),
(3, 'Data Structures', 'Collective note on common data structures and algorithms.', 'Professor Williams'),
(4, 'Computer Networks', 'Shared note for networking fundamentals and protocols.', 'Professor Brown'),
(5, 'Algorithms', 'Collaborative space for algorithm design and analysis.', 'Professor Davis');

-- Enroll users into classes (sample enrollments)
INSERT INTO user_classes (user_id, class_id) VALUES 
(1, 1), (1, 2), (1, 3), (1, 4), (1, 5),  -- Ensure test user is enrolled in all classes
(2, 1), (3, 1), (4, 1), (5, 1), (6, 1), (7, 1), (8, 1),
(3, 2), (5, 2), (7, 2),
(2, 3), (4, 3), (6, 3), (8, 3),
(1, 4), (2, 4), (4, 4), (5, 4), (7, 4),
(3, 5), (4, 5), (6, 5), (8, 5);

-- Insert shared notes for each class
INSERT INTO notes (class_id, title, file_path) VALUES 
(1, 'Operating Systems Shared Note', 'notes/class_1_note.txt'),
(2, 'Database Systems Shared Note', 'notes/class_2_note.txt'),
(3, 'Data Structures Shared Note', 'notes/class_3_note.txt'),
(4, 'Computer Networks Shared Note', 'notes/class_4_note.txt'),
(5, 'Algorithms Shared Note', 'notes/class_5_note.txt');