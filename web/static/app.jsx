class AvailablePedal extends React.Component {
  constructor(props) {
    super(props);
  }

  add() {
    $.get("/add_pedal/" + this.props.name, () => {
      this.props.onChange();
    });
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
    return this.props.pedals.map(pedal => {
      return (<AvailablePedal name={pedal} onChange={this.props.onChange} />)
    });
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
    this.state = {'pedals': []};
    this.props.registerRefresh(() => this.refresh());
  }

  componentDidMount() {
    this.refresh();
  }

  refresh() {
    $.get('/active_pedals').done(pedals => {
      this.setState({'pedals': pedals});
    });
  }

  render() {
    return this.state.pedals.map(pedal => {
      return <ActivePedal name={pedal.name} knobs={pedal.knobs} />
    });
  }
}

class App extends React.Component {
  constructor(props) {
    super(props);

    this.refreshHandlers = [];
  }

  registerRefreshHandler(handler) {
    this.refreshHandlers.push(handler);
  }

  refresh() {
    for (const refreshHandler of this.refreshHandlers) {
      refreshHandler();
    }
  }

  render() {
    return (
      <div>
        <PedalList pedals={["delay"]} onChange={this.refresh.bind(this)} />
        <hr />
        <PedalBoard registerRefresh={this.registerRefreshHandler.bind(this)} />
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

