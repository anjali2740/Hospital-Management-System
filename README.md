# 🏥 Hospital Management System

A simple yet functional **Hospital Management System** developed using **C (via C++)** and **MySQL**. This project helps hospitals manage patient records and appointments effectively from the command line.

---

## 🚀 Features

- ➕ Add Patient  
- 📋 View All Patients  
- ✏️ Edit Patient Details  
- 🔍 Search Patient by Name or ID  
- 📅 Schedule Appointment  
- 👁️ View Scheduled Appointments  

---

## 🛠️ Tech Stack

- **Language:** C (via `.cpp` in Dev C++)  
- **Database:** MySQL  
- **Software Used:** Dev C++, MySQL Server

---

## 📦 Installation & Setup

1. **Clone the Repository**
   ```bash
   git clone https://github.com/anjali2740/hospital-management-system.git
   cd hospital-management-system
2. **Set up the MySQL database**
   - Login to MySQL using your credentials.
   - Execute the following SQL commands:
     CREATE DATABASE hospital_db;
    USE hospital_db;

   CREATE TABLE patients (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(100),
    age INT,
    gender VARCHAR(10),
    contact VARCHAR(15),
    diagnosis VARCHAR(255));

    CREATE TABLE appointments (
    id INT AUTO_INCREMENT PRIMARY KEY,
    patient_id INT,
    appointment_date DATE,
    doctor_name VARCHAR(100),
    FOREIGN KEY (patient_id) REFERENCES patients(id));

3. **Configure Database Connection in Code:**
   Open hospital_management.cpp and update the MySQL connection string:
   ```bash
   mysql_real_connect(conn, "localhost", "root", "root", "hospital_db", 0, NULL, 0);

4. **Compile and Run:**
   Using Dev C++:
   - Open the hospital_management.cpp file.
   - Compile and run.
   Or using GCC:
   - gcc hospital_management.c

🙌 Contributions
Contributions are welcome!
Feel free to fork the repository and submit a pull request with improvements.

👩‍💻 Author
Angadi Anjali
📧 2210030411cse@gmail.com
🔗 LinkedIn : www.linkedin.com/in/angadi-anjali

📜 License
This project is licensed under the MIT License – see the LICENSE file for details.

   
