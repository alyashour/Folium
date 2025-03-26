<script lang="ts">
    import {
        Button,
        Col,
        Row,
        Dropdown,
        DropdownToggle,
        DropdownMenu,
        DropdownItem,
    } from "@sveltestrap/sveltestrap";

    // Optional state for dropdowns if needed in the future
    let textSizeDropdownOpen = false;
    let widthDropdownOpen = false;
    let colorDropdownOpen = false;

    function toggleTextSize() {
        textSizeDropdownOpen = !textSizeDropdownOpen;
    }
    function toggleWidth() {
        widthDropdownOpen = !widthDropdownOpen;
    }
    function toggleColor() {
        colorDropdownOpen = !colorDropdownOpen;
    }
</script>

<Row class="mt-3">
    <!-- Left Column: Class Selector -->
    <Col md="2">
        <h5>Classes</h5>
        <ul class="list-unstyled">
            <li><a href="/class1">Class 1</a></li>
            <li><a href="/class2">Class 2</a></li>
            <li><a href="/classn">Class n</a></li>
        </ul>
        <Button color="primary" size="sm">New Class</Button>
    </Col>

    <!-- Main Column: Article (stretches to min viewport height) -->
    <Col md="8">
        <h1>Article Title</h1>
        <p>
            Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed vitae
            sapien eget leo dapibus commodo. Quisque aliquam, sem ut interdum
            condimentum, velit quam tincidunt urna, ut fermentum nisl nisi ut
            est.
        </p>
    </Col>

    <!-- Right Column: Appearance Settings -->
    <Col md="2">
        <h5>Appearance</h5>

        <!-- Text Size Dropdown -->
        <Dropdown isOpen={textSizeDropdownOpen} toggle={toggleTextSize}>
            <DropdownToggle caret color="secondary" size="sm">
                Text Size
            </DropdownToggle>
            <DropdownMenu>
                <DropdownItem>Small</DropdownItem>
                <DropdownItem>Medium</DropdownItem>
                <DropdownItem>Large</DropdownItem>
            </DropdownMenu>
        </Dropdown>

        <!-- Width Dropdown -->
        <Dropdown isOpen={widthDropdownOpen} toggle={toggleWidth} class="mt-2">
            <DropdownToggle caret color="secondary" size="sm">
                Width
            </DropdownToggle>
            <DropdownMenu>
                <DropdownItem>Wide</DropdownItem>
                <DropdownItem>Narrow</DropdownItem>
            </DropdownMenu>
        </Dropdown>

        <!-- Color Dropdown -->
        <Dropdown isOpen={colorDropdownOpen} toggle={toggleColor} class="mt-2">
            <DropdownToggle caret color="secondary" size="sm">
                Color
            </DropdownToggle>
            <DropdownMenu>
                <DropdownItem>Automatic</DropdownItem>
                <DropdownItem>Light</DropdownItem>
                <DropdownItem>Dark</DropdownItem>
            </DropdownMenu>
        </Dropdown>
    </Col>
</Row>

<style>
    /* Remove list styling from the classes list */
    ul.list-unstyled {
        padding-left: 0;
    }
    ul.list-unstyled li a {
        text-decoration: none;
        color: inherit;
    }
    ul.list-unstyled li a:hover {
        text-decoration: underline;
    }
</style>
