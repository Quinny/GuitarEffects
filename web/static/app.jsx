class AvailablePedal extends React.Component {
  constructor(props) {
    super(props);
  }

  add() {
    // Call server to add pedal, refresh board
    console.log('Adding pedal');
  }

  render() {
    return (
      <p>
        {this.props.name} <button class="btn btn-primary btn-small"
                                  onClick={this.add.bind(this)}>Add</button>
      </p>
    )
  }
}

class PedalList extends React.Component {
  constructor(props) {
    super(props);
  }

  render() {
    return (<AvailablePedal name="Delay" />);
  }
}

class Knob extends React.Component {
  constructor(props) {
    super(props);
  }

  tweak() {
    console.log("Tweaking knob " + this.props.name);
  }

  render() {
    const boundTweak = this.tweak.bind(this);
    return (
      <p>
        {this.props.name}
        <button class="btn btn-primary btn-small" onClick={boundTweak}>
          +
        </button>
        <input value={this.props.value} onChange={boundTweak} />
        <button class="btn btn-primary btn-small" onClick={boundTweak}>
          -
        </button>
      </p>
    )
  }
}

class ActivePedal extends React.Component {
  constructor(props) {
    super(props);
  }

  render() {
    const knobs = this.props.knobs.map((knob) => {
      return (<Knob name={knob.name} value={knob.value} />)
    });
    return (
      <p>
        {this.props.name}
        {knobs}
      </p>
    )
  }
}

class PedalBoard extends React.Component {
  constructor(props) {
    super(props);
  }

  refresh() {
    // Call server and update state.
  }

  render() {
    return (
      <div>
        <ActivePedal name="Delay" knobs={[
          {
            "name": "amount",
            "value": 50,
          }
        ]}/>

        <ActivePedal name="Distortion" knobs={[
          {
            "name": "drive",
            "value": 50,
          }
        ]}/>
      </div>
    )
  }
}

class App extends React.Component {
  constructor(props) {
    super(props);
  }

  render() {
    return (
      <div>
        <PedalList />
        <hr />
        <PedalBoard />
      </div>
    )
  }
}

// Fetch the feed from the API and render it.
$(document).ready(() => {
  ReactDOM.render(
      <App />,
      document.getElementById('pedalboard')
  );
});

