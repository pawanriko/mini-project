const taskInput = document.getElementById("taskInput");
const addBtn = document.getElementById("addBtn");
const taskList = document.getElementById("taskList");
const taskCount = document.getElementById("taskCount");
const themeBtn = document.getElementById("themeBtn");

let tasks = JSON.parse(localStorage.getItem("tasks")) || [];
let currentFilter = "all";

function saveTasks() {
    localStorage.setItem("tasks", JSON.stringify(tasks));
}

function updateCounter() {
    taskCount.textContent = `Tasks: ${tasks.length}`;
}

function filterTasks(type) {
    currentFilter = type;
    renderTasks();
}

function renderTasks() {

    taskList.innerHTML = "";

    let filteredTasks = tasks;

    if (currentFilter === "completed") {
        filteredTasks = tasks.filter(task => task.completed);
    }

    if (currentFilter === "pending") {
        filteredTasks = tasks.filter(task => !task.completed);
    }

    filteredTasks.forEach((task) => {

        const li = document.createElement("li");

        if (task.completed) {
            li.classList.add("completed");
        }

        li.innerHTML = `
            <div>
                <span class="task-text">${task.text}</span>
                <br>
                <small>Priority: ${task.priority}</small>
                <br>
                <small>Due: ${task.dueDate || "Not Set"}</small>
            </div>

            <div class="actions">
                <button class="complete-btn">✔</button>
                <button class="delete-btn">✖</button>
            </div>
        `;

        li.querySelector(".complete-btn")
        .addEventListener("click", () => {
            task.completed = !task.completed;
            saveTasks();
            renderTasks();
        });

        li.querySelector(".delete-btn")
        .addEventListener("click", () => {
            tasks = tasks.filter(t => t !== task);
            saveTasks();
            renderTasks();
        });

        taskList.appendChild(li);
    });

    updateCounter();
}

function addTask() {

    const text = taskInput.value.trim();

    if (text === "") {
        alert("Please enter a task");
        return;
    }

    tasks.push({
        text: text,
        completed: false,
        priority: document.getElementById("priority").value,
        dueDate: document.getElementById("dueDate").value
    });

    saveTasks();

    taskInput.value = "";

    renderTasks();
}

addBtn.addEventListener("click", addTask);

taskInput.addEventListener("keypress", (e) => {
    if (e.key === "Enter") {
        addTask();
    }
});

themeBtn.addEventListener("click", () => {
    document.body.classList.toggle("light-mode");
});

renderTasks();